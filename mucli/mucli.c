/* mucli.c */
#include "mucli.h"
#include "account.h"
#include "log.h"

#include <stdio.h>

struct mucli mucli;

// Initilizers
static int init_state(int argc, char **argv);
static int init_mutex(int argc, char **argv);
static int init_connection(int argc, char **argv);
static int (*const init[])(int argc, char **argv) =
{
	init_state,
	init_mutex,
	init_connection,
};

// curl_share lock functions
static void curlsh_lock(CURL *, curl_lock_data, curl_lock_access, void *);
static void curlsh_unlock(CURL *, curl_lock_data, void *);

int
main(int argc, char **argv)
{
	// Initilize everything
	size_t i;
	for (i = 0; i < sizeof init / sizeof *init; ++i)
		if (init[i](argc, argv) != EXIT_SUCCESS)
			return EXIT_FAILURE;
	mucli_log(LOG_DEBUG,"debug");
	mucli_log(LOG_INFO ,"info");
	mucli_log(LOG_WARN ,"warn");
	mucli_log(LOG_ERROR,"error");
	mucli_log(LOG_FATAL,"fatal");
	if (argc >= 3)
		mucli_login(argv[1], argv[2]);
	if (mucli.account.state == LOGGED_IN) {
		mucli_log(LOG_DEBUG, "logged into %s", mucli.account.username);
		mucli_logout();
	}
}

// Init functions
int
init_state(int argc, char **argv)
{
	(void)argc, (void)argv;
	mucli.connection.max_tries = 10;

	mucli.account.state = LOGGED_OUT;
	mucli.account.username = NULL;
	return EXIT_SUCCESS;
}

int
init_mutex(int argc, char **argv)
{
	(void)argc, (void)argv;
	int err = 0;
	err |= pthread_mutex_init(&mucli.account.lock, NULL);
	err |= pthread_mutex_init(&mucli.connection.lock, NULL);
	if (err)
	{
		mucli_log(LOG_FATAL, "mutex init failed");
		if (errno)
			mucli_log(LOG_FATAL, "%s", strerror(errno));
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}

int
init_connection(int argc, char **argv)
{
#define setopt(type, opt) curl_share_setopt(mucli.connection.curlsh,\
		type, opt)
	(void)argc, (void)argv;
	CURLcode   code;
	CURLSHcode codesh;

	if ((code = curl_global_init(CURL_GLOBAL_ALL))
			|| !(mucli.connection.curlsh = curl_share_init( ))
			|| (codesh = setopt(CURLSHOPT_SHARE, CURL_LOCK_DATA_COOKIE))
			|| (codesh = setopt(CURLSHOPT_SHARE, CURL_LOCK_DATA_DNS))
			|| (codesh = setopt(CURLSHOPT_SHARE, CURL_LOCK_DATA_SSL_SESSION))
			|| (codesh = setopt(CURLSHOPT_LOCKFUNC, curlsh_lock))
			|| (codesh = setopt(CURLSHOPT_UNLOCKFUNC, curlsh_unlock)))
		goto fail;

	return EXIT_SUCCESS;
	fail:
	mucli_log(LOG_FATAL, "Failed to initialize curl %s",
		code ? curl_easy_strerror(code) :
		codesh ? curl_share_strerror(codesh) : "");
	return EXIT_FAILURE;
#undef setopt
}

// Lock functions

static const char * const lock_data_str[] =
{
	[CURL_LOCK_DATA_SHARE]       = "Share",
	[CURL_LOCK_DATA_COOKIE]      = "Cookie",
	[CURL_LOCK_DATA_DNS]         = "DNS",
	[CURL_LOCK_DATA_SSL_SESSION] = "SSL Session",
	[CURL_LOCK_DATA_CONNECT]     = "Connect",
};
static const char * const lock_access_str[] =
{
	[CURL_LOCK_ACCESS_NONE]   = "None",
	[CURL_LOCK_ACCESS_SHARED] = "Share",
	[CURL_LOCK_ACCESS_SINGLE] = "Single",
};

void
curlsh_lock(CURL *h, curl_lock_data d, curl_lock_access l, void *u)
{
	(void)h, (void)d, (void)l, (void)u;
	mucli_log(LOG_DEBUG,"%s:%s", lock_data_str[d], lock_access_str[l]);
	int err = pthread_mutex_lock(&mucli.connection.lock);
	if (err)
		mucli_log(LOG_ERROR, "mutex lock failure %s", strerror(err));
}

void
curlsh_unlock(CURL *h, curl_lock_data d, void *u)
{
	(void)h, (void)d, (void)u;
	mucli_log(LOG_DEBUG,"%s", lock_data_str[d]);
	int err = pthread_mutex_unlock(&mucli.connection.lock);
	if (err)
		mucli_log(LOG_ERROR, "mutex unlock failure %s", strerror(err));
}
