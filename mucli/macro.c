/* curl.c */
#include "macro.h"

size_t
_nowrite(char *p, size_t s, size_t n, void *u) { return n*s; }

