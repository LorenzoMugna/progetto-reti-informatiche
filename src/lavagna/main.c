#include "lavagna-utils.h"
#include "lavagna-state.h"
#include "lavagna-net.h"
#include "lavagna-cli.h"
#include "printing.h"
#include <signal.h>

int running;

void exit_handler(int sig)
{
	(void)(sig); // Evita warning parametro non usato
	running = 0; // Ferma il ciclo principale
}

const char *card_descriptions[] = {
	"Implementare la funzione di parsing dei comandi",
	"Implementare la gestione degli utenti",
	"Implementare la comunicazione di rete",
	"Implementare la CLI della lavagna",
	"Implementare la CLI dell'utente",
	"Implementare i timeout per le carte gestite dagli utenti",
	"Testare il progetto",
	"Debug dei messaggi con Wireshark",
	"Consegnare il progetto",
	"Sostenere esame di reti informatiche",
	NULL,
};

int create_cards()
{
	char buffer[256];
	for (int i = 0; card_descriptions[i] != NULL; i++)
	{
		memcpy(buffer, card_descriptions[i], strlen(card_descriptions[i]) + 1);
		card_t *card = new_card(last_card_id + 1, buffer);
		if (!card)
			return -1;
		last_card_id++;
		push_back(&to_do_list, &card->list);
	}
	return 0;
}

int main()
{
	signal(SIGPIPE, SIG_IGN);
	init_state();
	int server_socket = init_server();
	if (server_socket == -1)
	{
		fprintf(stderr, "Errore inizializzazione server");
		goto end;
	}

	int pipe = init_timeout_handler();
	if (pipe == -1)
	{
		fprintf(stderr, "Errore inizializzazione timeout handler");
		goto end;
	}


	if (create_cards() == -1)
	{
		fprintf(stderr, "Errore creazione carte di esempio");
		goto end;
	}

	signal(SIGINT, exit_handler);
	start_polling();

	init_printing();
	rewrite_prompt("Lavagna@5678");

	log_line("Lavagna inizializzata. Scriva HELP per una lista di comandi\n"
			 "eseguibili da terminale.\n");
	running = 1;
	// Ciclo principale della lavagna: attendi ricezione di un evento e gestiscilo
	while (running)
	{
		poll(sock_set, current_users + RESERVED_SOCK_SET_SOCKETS, -1);

		for (uint32_t i = 0; i < current_users + RESERVED_SOCK_SET_SOCKETS; i++)
		{
			if (sock_set[i].revents & (POLLERR | POLLHUP))
			{
				if (i >= RESERVED_SOCK_SET_SOCKETS)
				{
					user_t *user = find_user_from_fd(sock_set[i].fd);
					disconnect_user(user);
					i--; // Decrementa i per evitare di saltare l'elemento successivo dopo la rimozione
				}
				continue;
			}

			if (!(sock_set[i].revents & POLLIN))
				continue;

			int fd = sock_set[i].fd;

			switch (i)
			{
			case RESERVED_STDIN:
				if (cli_event() == -1)
					log_line("Errore nella gestione del comando\n");
				rewrite_prompt("Lavagna@5678");
				break;

			case RESERVED_COMMAND_PIPE:
				polling_handler();
				break;

			case RESERVED_LISTENER:
				int ret = accept_user(sock_set[i].fd);
				if (ret == -1)
				{
					log_line("Utente rifiutato.\n");
					break;
				}

				uint16_t new_user_port = (uint16_t)ret;
				log_line("Nuovo utente: %hu\n", new_user_port);
				break;

			default: // Gestione di un comando ricevuto da un utente
				user_t *user = find_user_from_fd(fd);
				uint16_t user_port = ntohs(user->sockaddr.sin_port);

				if (!user)
				{
					log_line("PANIC: non trovato l'utente associato al file descriptor %d", fd);
					goto end;
				}

				command_t *command;
				int status = recv_command(sock_set[i].fd, &command);

				if (status == -1)
				{
					log_line("Errore nella ricezione del comando da parte dell'utente %hu\n", user_port);
					disconnect_user(user);
					break;
				}

				if (!command)
				{
					log_line("Comando malformato\n");
					break;
				}

				if (network_handling_table[command->id])
					network_handling_table[command->id](user, command);

				destroy_command(command);
				break;
			}
		}
	}

end:
	stop_polling();
	destroy_timeout_handler();

	for (user_t *it = (user_t *)user_list.next; it != (user_t *)&user_list;)
	{
		user_t *next = (user_t *)it->list.next;
		disconnect_user(it);
		it = next;
	}

	clear_card_list(&to_do_list);
	clear_card_list(&doing_list);
	clear_card_list(&done_list);
	close(server_socket);

	end_printing(); // ripristina la finestra
	close(STDOUT_FILENO);
	return 0;
}