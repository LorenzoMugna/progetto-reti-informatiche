#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include <assert.h>
#include <signal.h>
#include <sys/poll.h>

#include "card.h"
#include "list.h"
#include "parsing.h"
#include "printing.h"
#include "utente-net.h"
#include "utente-cli.h"

typedef enum user_poll_reserved_fd
{
	RESERVED_STDIN,
	RESERVED_SOCKET,
	RESERVED_LISTENER,
	N_RESERVED
} user_reserved_fd_t;

struct pollfd topoll[N_RESERVED];

int running = 1;
void exit_handler(int sig)
{
	(void)(sig);
	running = 0;
}

int main(int argc, char **argv)
{
	signal(SIGINT, exit_handler);

	topoll[RESERVED_STDIN] = (struct pollfd){.fd = STDIN_FILENO, .events = POLLIN};

	short port = 0;
	if (argc == 2)
	{
		port = atoi(argv[1]);
	}

	if (port == 0)
	{
		port = 5679;
	}
	printf("%s\n%u\n", argv[1], port);

	int mysock = init_socket(port);
	if (mysock == -1)
		return 1;
	topoll[RESERVED_SOCKET] = (struct pollfd){.fd = mysock, .events = POLLIN};

	int linstener = init_listener_socket();
	if (linstener == -1)
		return 1;
	topoll[RESERVED_LISTENER] = (struct pollfd){.fd = linstener, .events = POLLIN};

	srand(time(NULL)+port);
	init_printing();
	rewrite_prompt("Utente@%hu", port);

	while(running)
	{
		poll(topoll, N_RESERVED, -1);
		if (topoll[RESERVED_STDIN].revents & POLLIN)
		{
			cli_event();
			rewrite_prompt("Utente@%hu", port);
		}
		if (topoll[RESERVED_SOCKET].revents & POLLIN)
		{
			net_event();
		}
		if (topoll[RESERVED_LISTENER].revents & POLLIN)
		{
			accept_request(linstener);
		}
	}


	clear_useraddr_list(&missing_reviews);
	destroy_card(handled_card);
	sendf(mysock, "%s\n\n", command_strings[QUIT]);
	end_printing();
	return 0;
}