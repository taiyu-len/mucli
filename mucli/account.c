/* account.c */
#include "account.h"
#include "mucli.h"
#include "log.h"
#include "macro.h"

char *
strdup(const char *);

void
mucli_login(const char *username, const char *password)
{
	int err;
	// Skip if logged in already
	if (mucli.account.state == LOGGED_IN)
		return;

	// Skip if login is locked
	if ((err = pthread_mutex_trylock(&mucli.account.lock)) == EBUSY)
		return;
	else if (err)
	{
		mucli_log(LOG_ERROR, "mutex locking failed %s", strerror(err));
		return;
	}
	mucli_log(LOG_INFO, "Logging into account %s", username);

	mucli.account.state = LOGGING_IN;
	mucli.account.username = strdup(username);
	long response;
	{
		// perform request to login
		CURLcode code = 0;
		CURL *eh = 0;
		size_t tries = 0;
		// Setup form information
		struct curl_httppost *post = NULL;
		struct curl_httppost *last = NULL;
		curl_form(PTR, "act", PTR, "login");
		curl_form(PTR, "username", COPY, username);
		curl_form(PTR, "password", COPY, password);
		while (++tries < mucli.connection.max_tries
				&& (curl_init(eh, LOGIN_URL, code)
				|| (code = curl_post(eh))
				|| (code = curl_easy_perform(eh))
				// Get response, (method to find if login succes or not)
				|| (code = curl_response(eh, response))))
		{
			mucli_log(LOG_ERROR, "Failed to connect: %s",
					code ? curl_easy_strerror(code): "");
			curl_easy_cleanup(eh);
		}
		curl_easy_cleanup(eh);
	}
	if (response == LOGIN_SUCCESS)
		mucli.account.state = LOGGED_IN;
	else
	{
		mucli.account.state = LOGGED_OUT;
		free((void *)mucli.account.username);
		mucli.account.username = NULL;
		mucli_log(LOG_INFO, "Login Failed");
	}

	if ((err = pthread_mutex_unlock(&mucli.account.lock)))
		mucli_log(LOG_ERROR, "mutex unlocking failed %s", strerror(err));
}

void
mucli_logout(void)
{
	int err;
	// Skip if already logged out.
	if (mucli.account.state == LOGGED_OUT)
		return;

	// Skip if account mutex is locked.
	if ((err = pthread_mutex_trylock(&mucli.account.lock)) == EBUSY)
		return;
	else if (err)
	{
		mucli_log(LOG_ERROR, "locking failed: %s", strerror(err));
		return;
	}
	mucli_log(LOG_INFO, "Logging out of %s", mucli.account.username);

	mucli.account.state = LOGGING_OUT;
	free((void *)mucli.account.username);
	mucli.account.username = NULL;
	{
		// perform request to logout
		CURLcode code = 0;
		CURL *eh = 0;
		size_t tries = 0;
		while (++tries < mucli.connection.max_tries
				&& (curl_init(eh, LOGOUT_URL, code)
				|| (code = curl_easy_perform(eh))))
			mucli_log(LOG_ERROR, "Failed to logout %s",
					code ? curl_easy_strerror(code): "");
		curl_easy_cleanup(eh);
	}
	mucli.account.state = LOGGED_OUT;

	if ((err = pthread_mutex_unlock(&mucli.account.lock)))
		mucli_log(LOG_ERROR, "mutex unlocking failed %s", strerror(err));
}


