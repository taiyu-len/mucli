/* mucli.c */
#include "mucli.h"
#include "init.h"
#include "macro.h"
#include "account.h"
#include "log.h"

#include <stdio.h>

struct mucli mucli;



int
main(int argc, char **argv)
{
	// Initilize everything
	if (mucli_init(argc, argv) != EXIT_SUCCESS)
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
	}

	mucli_clean();
}

// Init functions
// Lock functions

