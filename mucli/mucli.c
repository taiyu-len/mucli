/* mucli.c */
#include "mucli.h"
#include "init.h"
#include "macro.h"
#include "account.h"

#include <stdio.h>

struct mucli mucli;


int
main(int argc, char **argv)
{
	// Initilize everything
	if (mucli_init(argc, argv) != EXIT_SUCCESS)
		return EXIT_FAILURE;

	mucli_log(LOG_DEBUG, "(%dx%d) @ [%dx%d]",
			getmaxx(mucli.interface.win),
			getmaxy(mucli.interface.win),
			getbegx(mucli.interface.win),
			getbegy(mucli.interface.win)
			);

	if (argc >= 3)
		mucli_login(argv[1], argv[2]);
	else
		mucli_log(LOG_WARN, "To use (for now) run program with arguments "
				"username password");
	if (mucli.account.state == LOGGED_IN) {
		mucli_log(LOG_DEBUG, "logged into %s", mucli.account.username);
		mucli_logout();
	}

	mucli_clean();
}

