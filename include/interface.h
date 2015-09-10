#ifndef  _INTERFACE_H
#define  _INTERFACE_H
#include <ncurses.h>
#include <stddef.h>

// Generic containers used for easy layout management
union box;
enum type { CONT, WIN } type;

struct common
{
	enum type type;
	struct
	{
		short x, y;
		short w, h;
	}
	size;
	short weight;
};

struct container
{
	enum type type;
	struct
	{
		short x, y;
		short w, h;
	}
	size;
	short weight;
	// Whether the boundry between children are on a ROW or COL
	enum split { ROW,  COL } split;
	char  length;
	union box **child;
};

struct window
{
	enum type type;
	struct
	{
		short x, y;
		short w, h;
	}
	size;
	short weight;
	WINDOW *ptr;
	int (*redraw)(struct window *);
};

union box
{
	enum type type;
	struct common    common;
	struct window    window;
	struct container container;
};

// Interface  structure

struct interface
{
	// Main window returned by initscr()
	WINDOW *win;

	// Main container
	struct container main;

	// mucli_log prints here.
	struct
	{
		struct window win;
		enum verbosity
		{
			LOG_DEBUG,
			LOG_INFO,
			LOG_WARN,
			LOG_ERROR,
			LOG_FATAL,
			// Unused
			LOG_LAST,
		}
		verbosity;
	}
	log;
};

int
init_interface(int argc, char **argv);

void
clean_interface(void);

void __attribute__((format(printf, 3, 4)))
_mucli_log(enum verbosity,const char *func, const char *str, ...);

#define mucli_log(v, str, ...) \
	_mucli_log(v, __PRETTY_FUNCTION__, str, ##__VA_ARGS__)

#endif /*_INTERFACE_H*/

