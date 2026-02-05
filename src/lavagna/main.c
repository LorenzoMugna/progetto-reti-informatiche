#include "state-handler.h"
#include "lavagna-net.h"
#include "printing.h"
#include <signal.h>
#include <setjmp.h>

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
		log_line("%d: %s (%d content length)\n", test->id, str_command_tokens[test->id], strlen(test->content));
	}
	else
	{
		log_line("Not found.\n");
	}

	log_line("%s\n", test->content);

	destroy_command(test);
}

void free_card_list(list_t *list){
	while(!list_empty(list))
	{
		card_t *card = (card_t*)list->next;
		destroy_card(card);
	}
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
	longjmp(exit_jump_buffer, 1); //Longjmp alla fine del main
}

int main()
{

	init_state();
	init_printing();
	int ser_sock = init_server();

	if (ser_sock == -1)
	{
		fprintf(stderr, "Errore inizializzazione server");
		exit(1);
	}

	if(setjmp(exit_jump_buffer) == 1)
		goto end;
	signal(SIGINT, exit_handler);

	// Ciclo principale della lavagna: attendi ricezione di un evento e gestiscilo
	while (1)
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
			case RESERVED_COMMAND_PIPE:
				char buf[1024];
				int n = read(STDIN_FILENO, buf, sizeof(buf));
				buf[n] = '\0';
				log_line("Ricevuto: '%s', ma devo ancora implementare il resto...\n", buf);
				rewrite_prompt();
				// TODO
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
				log_line("New User! (%d) %s:%u\n", current_users, netaddr, port);
				break;

			default: // Gestione di un comando ricevuto da un utente

				int command_id = -1;
				command_t *command = recv_command(sock_set[i].fd);
				if (!command)
				{
					fprintf(stderr, "Errore nella struttura del comando");
					user_t *user = find_user_from_fd(sock_set[i].fd);
					disconnect_user(user);
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
					fprintf(stderr, "non trovato l'handler associato al comando %s", str_command_tokens[command_id]);
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
	free_card_list(&to_do_list);
	free_card_list(&doing_list);
	free_card_list(&done_list);
	end_printing(); // ripristina la finestra
	return 0;
}