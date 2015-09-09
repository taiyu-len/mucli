#ifndef  _MUCLI_H
#define  _MUCLI_H
#include <curl/curl.h>
#include <pthread.h>
#include <errno.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include "interface.h"
#include "attr.h"

// Program state
extern struct mucli
{
	struct
	{
		pthread_mutex_t lock;
		CURLSH *curlsh;
		size_t max_tries;
	}
	connection;

	struct
	{
		pthread_mutex_t lock;
		enum
		{
			// Login State
			LOGGED_IN,
			LOGGED_OUT,
			LOGGING_IN,
			LOGGING_OUT,
			// Response codes
			LOGIN_SUCCESS = 302,
		}
		volatile state;
		const char *username;
	}
	account;

	struct interface interface;
}
mucli;

// Common strings
#define DOMAIN_URL "https://www.mangaupdates.com/"
#define LOGIN_URL  DOMAIN_URL "login.html"
#define LOGOUT_URL DOMAIN_URL "login.html?act=logout"
#define LIST_URL   DOMAIN_URL "mylist.html"

#endif /*_MUCLI_H*/

