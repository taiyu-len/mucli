#ifndef  _ACCOUNT_H
#define  _ACCOUNT_H
#include "mucli.h"

void NONULL
mucli_login(const char *username, const char *password);

void
mucli_logout(void);

#endif /*_ACCOUNT_H*/

