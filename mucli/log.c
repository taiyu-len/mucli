/* log.c */
#include "log.h"

char const*const
logcolor[LOG_LAST] =
{
#define BG "[40;1;"
#define END "[49m"
	[LOG_DEBUG] = BG "32m" "debug " END,
	[LOG_INFO]  = BG "36m" "info  " END,
	[LOG_WARN]  = BG "33m" "warn  " END,
	[LOG_ERROR] = BG "31m" "error " END,
	[LOG_FATAL] = BG "4m"  "fatal " END,
};
