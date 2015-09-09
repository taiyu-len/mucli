/* log.c */
#include "log.h"

char const * const logcolor[LOG_LAST] =
{
	[LOG_DEBUG] = "[40;1;32m" "debug [49m",
	[LOG_INFO]  = "[40;1;36m" "info  [49m",
	[LOG_WARN]  = "[40;1;33m" "warn  [49m",
	[LOG_ERROR] = "[40;1;31m" "error [49m",
	[LOG_FATAL] = "[40;1;4m"  "fatal [49m",
};
