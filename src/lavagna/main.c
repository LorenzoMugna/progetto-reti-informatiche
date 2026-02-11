#include "lavagna-utils.h"
#include "lavagna-state.h"
#include "lavagna-net.h"
#include "lavagna-cli.h"
#include "printing.h"
#include <signal.h>
#include <setjmp.h>


int running;

void parse_test()
{
	char testcmnd[] = "HELLO \n\n";
	char temp[sizeof(testcmnd)];
	memcpy(temp, testcmnd, strlen(testcmnd) + 1);
	// test memory leak
	command_t *test = parse_command(temp);
	destroy_command(test);

	memcpy(temp, testcmnd, strlen(testcmnd) + 1);
	test = parse_command(temp);

	if (test)
	{
		log_line("%d: %s (%d content length)\n", test->id, command_strings[test->id], strlen(test->content));
	}
	else
	{
		log_line("Not found.\n");
	}

	log_line("%s\n", test->content);

	destroy_command(test);
}


void card_test()
{

	char msg[] = "Hello world!";
	for (int i = 0; i < 3; i++)
	{
		card_t *elem = malloc(sizeof(*elem));
		elem->ID = i + 1;
		elem->desc = msg;

		push_back(&to_do_list, &elem->list);
	}
	show_lavagna_handler();

	list_t *elem = pop_back(&to_do_list);
	push_back(&done_list, elem);
	show_lavagna_handler();
}

jmp_buf exit_jump_buffer;
void exit_handler(int sig)
{
	(void)(sig);	// Evita warning parametro non usato
	running = 0;	// Ferma il ciclo principale
}

int main()
{
	init_state();
	int server_socket = init_server();
	if (server_socket == -1)
	{
		fprintf(stderr, "Errore inizializzazione server");
		exit(1);
	}

	int pipe = init_timeout_handler();
	if (pipe == -1)
	{
		fprintf(stderr, "Errore inizializzazione timeout handler");
		exit(1);
	}

	signal(SIGINT, exit_handler);
	start_polling();

	init_printing();
	rewrite_prompt("Lavagna@5678");
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
				}
				continue;
			}

			if (!(sock_set[i].revents & POLLIN))
				continue;

			int fd = sock_set[i].fd;

			switch (i)
			{
			case RESERVED_STDIN:
				if(cli_event()==-1)
					log_line("Errore nella gestione del comando\n");
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

				user_t *last_user = (user_t *)user_list.prev;
				uint16_t port = ntohs(last_user->sockaddr.sin_port);
				char netaddr[20];
				inet_ntop(AF_INET, &last_user->sockaddr.sin_addr, netaddr, sizeof(netaddr));
				log_line("New User! (%d) %s:%hu\n", current_users, netaddr, port);
				break;

			default: // Gestione di un comando ricevuto da un utente
				int command_id = -1;
				command_t *command;
				int status = recv_command(sock_set[i].fd, &command);
				if (status == -1)
				{
					log_line("Errore nella ricezione del comando da parte dell'utente associato al file descriptor %d\n", fd);
					disconnect_user(find_user_from_fd(fd));
					break;
				}

				if (!command)
				{
					log_line("Comando malformato\n");
					break;
				}

				command_id = command->id;
				user_t *user = find_user_from_fd(fd);
				if (!user)
				{
					log_line("PANIC: non trovato l'utente associato al file descriptor %d", fd);
					goto end;
				}

				if (!network_handling_table[command_id])
				{
					fprintf(stderr, "non trovato l'handler associato al comando %s", command_strings[command_id]);
				}
				else
				{
					network_handling_table[command_id](user, command);
				}

				destroy_command(command);
				break;
			}
		}
	}

	//Routine di uscita, raggiunta tramite longjmp (vedi exit_handler)
end:
	for(user_t *it = (user_t *)user_list.next; it != (user_t *)&user_list; )
	{
		user_t *next = (user_t *)it->list.next;
		disconnect_user(it);
		it = next;
	}

	clear_card_list(&to_do_list);
	clear_card_list(&doing_list);
	clear_card_list(&done_list);
	destroy_timeout_handler();
	close(server_socket);

	end_printing(); // ripristina la finestra
	close(STDOUT_FILENO);	
	return 0;
}