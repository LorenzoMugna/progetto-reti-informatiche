#include "lavagna-timeout.h"
#include "lavagna-state.h"
#include "lavagna-net.h"
#include "printing.h"

#include <signal.h>

int timeout_pipe[2];

int init_timeout_handler()
{
	if (pipe(timeout_pipe) == -1)
		goto error;

	sock_set[RESERVED_COMMAND_PIPE] = (struct pollfd){.fd = timeout_pipe[0],
													  .events = POLLIN};
	return timeout_pipe[0];

error:
	return -1;
}

void destroy_timeout_handler()
{
	close(timeout_pipe[0]);
	close(timeout_pipe[1]);
	memset(&sock_set[RESERVED_COMMAND_PIPE], 0, sizeof(sock_set[0]));
}

void signal_polling_in_pipe(int sig)
{
	(void)sig;
	if (write(timeout_pipe[1], "x", 1) == -1)
		perror("write");

	alarm(POLLING_PERIOD);
}

void start_polling()
{
	signal(SIGALRM, signal_polling_in_pipe);
	alarm(POLLING_PERIOD);
}

void stop_polling()
{
	signal(SIGALRM, SIG_IGN);
	alarm(0);
}

void polling_handler()
{
	char discard_buf;
	read(timeout_pipe[0], &discard_buf, sizeof(discard_buf)); 

	// Scorri gli utenti e controlla se qualcuno è scaduto
	for (list_t *iter = user_list.next; iter != &user_list; iter = iter->next)
	{
		user_t *u = (user_t *)iter;

		if (u->timeout_type == TIMEOUT_NONE)
			continue;

		time_t now = time(NULL);

		if (u->next_timeout > now)
			continue;

		uint16_t user_port = ntohs(u->sockaddr.sin_port);

		switch (u->timeout_type)
		{
		case TIMEOUT_ACK_CARD:
			log_line("Carta non accettata in tempo da %hu\n", user_port);
			card_t *card = u->handled_card;
			if (card == NULL)
				break;
			card->user = 0; // Rilascia la carta
			u->handled_card = NULL;
			break;

		case TIMEOUT_PING_USER:
			log_line("[PING_USER] %hu\n", user_port);
			int fd = u->socket;
			sendf(fd, "%s ", command_strings[PING_USER]);
			u->next_timeout = now + PING_TIMEOUT;
			u->timeout_type = TIMEOUT_PONG_LAVAGNA;
			break;

		case TIMEOUT_PONG_LAVAGNA:
			disconnect_user(u);
			break;

		default:
			break;
		}
	}
}