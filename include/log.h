#ifndef  _LOG_H
#define  _LOG_H

enum {
	LOG_DEBUG,
	LOG_INFO,
	LOG_WARN,
	LOG_ERROR,
	LOG_FATAL,
	LOG_LAST,
};
extern char const * const logcolor[LOG_LAST];

// TODO much better logging
#define mucli_log(v, str, ...) \
	fprintf(stderr, "%s" str "[0m\n", logcolor[v], ##__VA_ARGS__)



#endif /*_LOG_H*/

