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

int running;
void exit_handler(int sig)
{
	(void)(sig);
	running = 0;
}

int main(int argc, char **argv)
{
	signal(SIGINT, exit_handler);
	signal(SIGPIPE, SIG_IGN); // Ignora SIGPIPE, gli errori sono già gestiti

	short port = 0;
	if (argc == 2)
	{
		port = atoi(argv[1]);
	}

	if (port <= 1024 || port == 5678) // Impedisci utilizzo delle well-known port e della porta 5678
	{
		port = 5679;
	}

	topoll[RESERVED_STDIN] = (struct pollfd){.fd = STDIN_FILENO, .events = POLLIN};

	int mysock = init_socket(port);
	if (mysock == -1)
	{
		printf("Errore inizializzazione socket verso la lavagna.\n"
			   "Assicurarsi che nessun altro utente con la stessa porta sia in esecuzione.\n");
		return 1;
	}
	topoll[RESERVED_SOCKET] = (struct pollfd){.fd = mysock, .events = POLLIN};

	int listener = init_listener_socket();
	if (listener == -1)
	{
		printf("Errore inizializzazione socket di ascolto.\n");
		return 1;
	}
	topoll[RESERVED_LISTENER] = (struct pollfd){.fd = listener, .events = POLLIN};

	// Randomizza includendo il numero di porta per utenti
	// che vengono fatti partire nello stesso secondo.
	srand(time(NULL) + port);

	init_printing();
	rewrite_prompt("Utente@%hu", port);

	log_line("Utente inizializzato sulla porta %hu. Scriva HELP per una lista di comandi\n"
			 "eseguibili da terminale.\n",
			 port);

	running = 1;
	while (running)
	{
		poll(topoll, N_RESERVED, -1);
		if (topoll[RESERVED_STDIN].revents & POLLIN)
		{
			cli_event();
			rewrite_prompt("Utente@%hu", port);
		}

		if (topoll[RESERVED_SOCKET].revents & POLLIN)
			net_event();

		if (topoll[RESERVED_LISTENER].revents & POLLIN)
			accept_request(listener);
	}

	clear_useraddr_list(&missing_reviews);
	destroy_card(handled_card);
	sendf(mysock, "%s\n\n", command_strings[QUIT]);
	end_printing();
	return 0;
}