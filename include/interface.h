#ifndef  _INTERFACE_H
#define  _INTERFACE_H
#include <ncurses.h>
#include <stddef.h>

struct interface
{
	WINDOW *win;
	// Small window on bottom of screen that shows logs
	struct
	{
		WINDOW *win;
		size_t h;
		// What to show in log
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

