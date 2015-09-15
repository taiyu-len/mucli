/* interface.c */
#include "interface.h"
#include "mucli.h"
#include <stdlib.h>
#include <stdarg.h>
#include <time.h>



// -Layout-
//
// |--------|
// |        |
// |  TODO  |
// |        |
// |--------|
// |LOGGING |
// |--------|

#define CLR 16
#define PAIR(fg, bg) COLOR_PAIR((bg)*CLR + (fg))
#define BRIGHT(c) ((c)+8)

// Container management functions
static struct container * USERET
new_container(enum split split, short weight)
{
	struct container *cont = malloc(sizeof(struct container));
	cont->type   = CONT;
	cont->split  = split;
	cont->weight = weight;
	cont->length = 0;
	cont->child  = NULL;
	return cont;
}

static void NONULL
free_container(struct container *cont)
{
	free(cont->child);
	free(cont);
}

static void NONULL
add_child(struct container *cont, box_ptr_t box)
{
	void *tmp = realloc(cont->child, ++cont->length * sizeof(*cont->child));
	if (tmp)
	{
		cont->child = tmp;
		cont->child[cont->length - 1] = box;
		return;
	}
	mucli_log(LOG_FATAL, "reallocation Failed");
}



// window functions
static WINDOW *
window_init(struct window *window)
{
	WINDOW *win = newwin(window->size.h, window->size.w,
			window->size.y, window->size.x);
	if (win)
	{
		mucli_log(LOG_DEBUG, "Creating window");
		return win;
	}
	mucli_log(LOG_FATAL, "Failed to create window");
}

static bool
window_update(struct window *window)
{
	short x, y;
	short h, w;
	getbegyx(window->ptr, y, x);
	getmaxyx(window->ptr, h, w);
	bool ch = false;
	mucli_log(LOG_DEBUG,"(%d,%d)@(%dx%d)->(%dx%d)@(%dx%d)",
			h, w, y, x,
			window->size.h, window->size.w, window->size.y, window->size.x);
	if ((ch |= (window->size.h > h && window->size.y != y)))
		mvwin(window->ptr, y = window->size.y, x);
	if ((ch |= (window->size.h != h)))
		wresize(window->ptr, h = window->size.h, w);
	if ((ch |= (window->size.y != y)))
		mvwin(window->ptr, y = window->size.y, x);

	if ((ch |= (window->size.w > w && window->size.x != x)))
		mvwin(window->ptr, y, x = window->size.x);
	if ((ch |= (window->size.w != w)))
		wresize(window->ptr, h, w = window->size.w);
	if ((ch |= (window->size.x != x)))
		mvwin(window->ptr, y, x = window->size.x);
	return ch;
}

// Redraw functions

static int
redraw_log(struct window *log)
{
	if (!log->ptr)
		log->ptr = window_init(log);
	else if (!window_update(log))
		return EXIT_SUCCESS;
	// Set log values
	if (scrollok(log->ptr, TRUE) == ERR)
	{
		mucli_log(LOG_FATAL, "Scrollok failed");
		return EXIT_FAILURE;
	}
	wnoutrefresh(log->ptr);
	return EXIT_SUCCESS;
};
// Layout functions

static int NONULL
_update_layout(box_ptr_t box)
{
	mucli_log(LOG_DEBUG, "(%dx%d)@(%dx%d)",
			box.common->size.h, box.common->size.w,
			box.common->size.y, box.common->size.x);
	if (*box.type == WIN)
		return box.window->redraw(box.window);

	if (*box.type != CONT)
	{ // Should never happen
		mucli_log(LOG_FATAL, "Something very wrong happened");
	}

	struct container *cont = box.container;
	if (cont->length == 0)
		return EXIT_SUCCESS;

	int i;
	// find total weight
	short weight = 1;
	for (i = 0; i < cont->length; ++i)
		weight += cont->child[i].common->weight;

	if (cont->split == COL)
	{
		// Split along column
		short x = cont->size.x;
		short y = cont->size.y;
		short w = cont->size.w - (cont->length - 1);
		short h = cont->size.h;
		short t = 0;
		for (i = 0; i < cont->length; ++i)
		{ // Get width
			box_ptr_t child = cont->child[i];
			child.common->size.y = y;
			child.common->size.h = h;
			t += child.common->size.w = (w * child.common->weight) / weight;
		}
		{ // add extra lines
			w -= t;
			for (i = 0; i < w; ++i)
				cont->child[i % cont->length].common->size.w++;
		}
		for (i = 0; i < cont->length; ++i)
		{ // set x values
			box_ptr_t child = cont->child[i];
			child.common->size.x = x;
			x += child.common->size.w + 1;
			mvvline(y, x-1, '|', h);
			_update_layout(child);
		}
	}
	else if (cont->split == ROW)
	{
		// Split along rows
		short x = cont->size.x;
		short y = cont->size.y;
		short w = cont->size.w;
		short h = cont->size.h - (cont->length - 1);
		short t = 0;
		for (i = 0; i < cont->length; ++i)
		{
			box_ptr_t child = cont->child[i];
			child.common->size.x = x;
			child.common->size.w = w;
			t += child.common->size.h = (h * child.common->weight) / weight;
		}
		// add extra lines
		h -= t;
		for (i = 0; i < h; ++i)
			cont->child[i % cont->length].common->size.h++;

		// set y values and recurse for children
		for (i = 0; i < cont->length; ++i)
		{
			box_ptr_t child = cont->child[i];
			child.common->size.y = y;
			y += child.common->size.h + 1;
			mvwhline(mucli.interface.win, y-1, x, '-', w);
			_update_layout(child);
		}
	}
	else
		return EXIT_FAILURE;
	return EXIT_SUCCESS;
}

static int NONULL
update_layout(box_ptr_t box)
{
	int retval = _update_layout(box);
	wnoutrefresh(mucli.interface.win);
	doupdate();
	return retval;
}

static int
init_layout(int argc, char **argv)
{
	struct interface *const IF = &mucli.interface;
	{ /* Initilize main container */
		struct container *tmp = new_container(ROW, 0);
		IF->main = *tmp;
		free_container(tmp);
		getbegyx(IF->win, IF->main.size.x, IF->main.size.y);
		getmaxyx(IF->win, IF->main.size.h, IF->main.size.w);
		add_child(&IF->main, new_container(COL, 8));
		add_child(&IF->main, &IF->log.win);
	}
	{ /* Initilize log container */
		IF->log.win.ptr    = NULL;
		IF->log.win.type   = WIN;
		IF->log.win.weight = 2;
		IF->log.win.redraw = redraw_log;
		redraw_log(&IF->log.win);
	}

	if (update_layout(&IF->main) != EXIT_SUCCESS)
		goto fail;
	return EXIT_SUCCESS;
	fail:
	fprintf(stderr, "Fatal: failed to initialize layout\n");
	return EXIT_FAILURE;
}

int
init_interface(int argc, char **argv)
{
	// Initialize ncurses
	if ((mucli.interface.win = initscr()) == NULL)
		goto fail;

	if (// Color options
			has_colors() == TRUE
			&& (start_color() == ERR
				|| use_default_colors() == ERR))
		goto fail;
	{
		// Set Colors
		short f, b;
		for (b = 0; b < CLR; ++b)
			for (f = 0; f < CLR; ++f)
				if (init_pair(b*CLR + f, f, b) == ERR)
					goto fail;
	}

	if (// Input options
			cbreak() == ERR
			|| noecho() == ERR
			|| keypad(stdscr, TRUE) == ERR
			|| nonl() == ERR
			|| intrflush(stdscr, FALSE) == ERR
			|| curs_set(0) == ERR)
		goto fail;

	// Logging
	// TODO set via argv
	mucli.interface.log.verbosity = LOG_DEBUG;

	init_layout(argc, argv);

	return EXIT_SUCCESS;
	fail:
	fprintf(stderr, "Fatal: failed to initialize ncurses");
	return EXIT_FAILURE;
}

void
clean_interface(void)
{
	//TODO close message popup
	getch();
	endwin();
	nl();
}


void
_mucli_vlog(enum verbosity v, const char *func, const char *str, va_list ap)
{
	static struct log_type {
		char  const *const str;
		short const fg;
		int   const intesity;
	} log_type[LOG_LAST] =
	{
		[LOG_DEBUG] = {"Debug ", COLOR_GREEN         , A_BOLD},
		[LOG_INFO]  = {"Info  ", BRIGHT(COLOR_BLUE)  , A_BOLD},
		[LOG_WARN]  = {"Warn  ", BRIGHT(COLOR_YELLOW), A_BOLD},
		[LOG_ERROR] = {"Error ", BRIGHT(COLOR_RED)   , A_BOLD},
		[LOG_FATAL] = {"Fatal ", COLOR_WHITE         , A_BOLD},
	};
	if (v < mucli.interface.log.verbosity)
		return;

	else if (v >= LOG_LAST)
	{
		mucli_log(LOG_ERROR, "Invalid verbosity level %d", v);
		return;
	}
	// Time
	time_t _now = time(0);
	char now[8];
	strftime(now, 9, "%H:%M:%S", localtime(&_now));

	if (mucli.interface.log.win.ptr)
	{ /* Print to ncurses log window */
		WINDOW *win = mucli.interface.log.win.ptr;
		if (// Print log to screen, and scroll old log down
				wmove(win, 0, 0) == ERR
				|| wscrl(win, -1) == ERR
				// Print log type
				|| wattron(win, PAIR(log_type[v].fg, COLOR_BLACK)) == ERR
				|| wattron(win, log_type[v].intesity) == ERR
				|| wprintw(win, log_type[v].str) == ERR
				|| wattron(win, PAIR(log_type[v].fg, BRIGHT(COLOR_BLACK))) == ERR
				// Print function name
				|| wprintw(win, " %8s | %-32s", now, func) == ERR
				|| wattroff(win, PAIR(log_type[v].fg, BRIGHT(COLOR_BLACK))) == ERR
				// Print log
				|| wprintw(win, " ") == ERR
				|| vwprintw(win, str, ap) == ERR
				|| wattroff(win, log_type[v].intesity) == ERR
				|| wrefresh(win) == ERR)
			wprintw(win, "Failed to print log");
	}
	else
	{ /* Print to stderr */
		// Time
		time_t _now = time(0);
		char now[8];
		strftime(now, 9, "%H:%M:%S", localtime(&_now));
		fprintf(stderr, "%s", log_type[v].str);
		fprintf(stderr, " %8s | %-32s", now, func);
		vfprintf(stderr, str, ap);
	}
}

void
_mucli_log(enum verbosity v, const char *func, const char *str, ...)
{
	va_list ap;
	va_start(ap, str);
	_mucli_vlog(v, func, str, ap);
	va_end(ap);
}

void
_mucli_fatal(const char *func, const char *str, ...)
{
	va_list ap;
	va_start(ap, str);
	_mucli_vlog(LOG_FATAL, func, str, ap);
	va_end(ap);
	exit(-1);
}

