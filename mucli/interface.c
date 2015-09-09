/* interface.c */
#include "interface.h"
#include "mucli.h"
#include <stdlib.h>
#include <stdarg.h>
#include <time.h>

#define CLR 16
#define PAIR(fg, bg) COLOR_PAIR((bg)*CLR + (fg))
#define BRIGHT(c) ((c)+8)

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
			|| intrflush(stdscr, FALSE) == ERR)
		goto fail;

	// Logging
	WINDOW *win;
	size_t h;
	mucli.interface.log.verbosity = LOG_DEBUG;
	mucli.interface.log.h = h = 10;
	if ((mucli.interface.log.win = win = newwin(h, 0, LINES - h, 0)) == NULL)
		goto fail;

	if (// Logging setup
			scrollok(win, TRUE) == ERR
			|| mvhline(LINES - h - 1, 0, '-', 1000) == ERR
			|| refresh() == ERR)
		goto fail;

	return EXIT_SUCCESS;
	fail:
	fprintf(stderr, "Fatal: failed to initialize ncurses");
	return EXIT_FAILURE;
}

void
clean_interface(void)
{
	getch();
	endwin();
	nl();
}

void
_mucli_log(enum verbosity v, const char *func, const char *str, ...)
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
	if (v >= LOG_LAST)
		mucli_log(LOG_ERROR, "Invalid verbosity level %d", v);
	else
	{
		va_list args;
		va_start(args, str);
		// move curser to window
		WINDOW *win = mucli.interface.log.win;
		// Time
		time_t _now = time(0);
		char now[8];
		strftime(now, 9, "%H:%M:%S", localtime(&_now));
		if (// Print log to screen, and scroll old log down
				wmove(win, 0, 0) == ERR
				|| wscrl(win, -1) == ERR
				// Print log type
				|| wattron(win, PAIR(log_type[v].fg, COLOR_BLACK)) == ERR
				|| wattron(win, log_type[v].intesity) == ERR
				|| wprintw(win, log_type[v].str) == ERR
				|| wattron(win, PAIR(log_type[v].fg, BRIGHT(COLOR_BLACK))) == ERR
				// Print function name
				|| wprintw(win, "%9s | %-32s", now, func) == ERR
				|| wattroff(win, PAIR(log_type[v].fg, BRIGHT(COLOR_BLACK))) == ERR
				// Print log
				|| wprintw(win, " ") == ERR
				|| vwprintw(win, str, args) == ERR
				|| wattroff(win, log_type[v].intesity) == ERR
				|| wrefresh(win) == ERR)
			wprintw(win, "Failed to print log");

		// Cleanup
		va_end(args);
	}
}

