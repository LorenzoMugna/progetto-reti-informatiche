#include <signal.h>

#include "state-handler.h"
#include "lavagna-net.h"
#include "printing.h"

/* -------------- Definizione entità globali ------------------*/
/* --(visibili in tutte le unità di compilazione di lavagna) -- */

list_t to_do_list;
list_t doing_list;
list_t done_list;

list_t user_list;

uint64_t last_card_id;

user_list_t *user_table[MAX_PORT];

void print_cardlist(list_t *b)
{
	if (!b || list_empty(b))
	{
		log_line("\n");
		return;
	}

	list_t *iter = b->next;
	while (b != iter)
	{
		card_list_t *cardp = (card_list_t *)iter;
		card_t *curcard = &cardp->card;
		log_line("\t%lu: %s\n", curcard->ID, curcard->mess);
		iter = iter->next;
	}

	// log_line("\n");
}

void show_lavagna_handler()
{
	log_line("To Do:\n");
	print_cardlist(&to_do_list);
	log_line("Doing:\n");
	print_cardlist(&doing_list);
	log_line("Done:\n");
	print_cardlist(&done_list);
	log_line("-----------------------------\n");
}

void parse_test()
{
	char testcmnd[] = "HELLO \n\n";
	char temp[strlen(testcmnd) + 1];
	memcpy(temp, testcmnd, strlen(testcmnd) + 1);
	// test memory leak
	command_t *test = parse_command(temp);
	destroy_command_list(test);
	free(test);

	memcpy(temp, testcmnd, strlen(testcmnd) + 1);
	test = parse_command(temp);

	if (test)
	{
		log_line("%d: %s (%d arg)\n", test->command, str_command_tokens[test->command], test->argc);
	}
	else
	{
		log_line("Not found.\n");
	}

	list_t *iter = test->param_list.next;
	while (iter != &test->param_list)
	{
		command_arg_list_t *elem = (command_arg_list_t *)iter;
		log_line("%s\n", elem->buffer);
		iter = iter->next;
	}

	destroy_command_list(test);
	free(test);
}

void card_test()
{

	char msg[] = "Hello world!";
	for (int i = 0; i < 3; i++)
	{
		card_list_t *elem = malloc(sizeof(*elem));
		elem->card.ID = i + 1;
		elem->card.mess = msg;

		push_back(&to_do_list, &elem->list_elem);
	}
	show_lavagna_handler();

	list_t *elem = pop_back(&to_do_list);
	push_back(&done_list, elem);
	show_lavagna_handler();
}

void exit_handler(int sig)
{
	(void)(sig);	// Evita warning parametro non usato
	end_printing(); // ripristina la finestra
	exit(0);
}

int main()
{
	init_printing();
	signal(SIGINT, exit_handler);

	// Initing server state variables
	init_list(&to_do_list);
	init_list(&doing_list);
	init_list(&done_list);
	init_list(&user_list);
	last_card_id = 0;

	int ser_sock = init_server();
	if (ser_sock == -1)
	{
		fprintf(stderr, "Errore inizializzazione server");
		exit(1);
	}
	while (1)
	{
		poll(sock_set, current_users + RESERVED_SOCK_SET_SOCKETS, -1);
		for (uint32_t i = 0; i < current_users + RESERVED_SOCK_SET_SOCKETS; i++)
		{
			if (sock_set[i].revents & (POLLERR | POLLHUP))
			{
				if (i >= RESERVED_SOCK_SET_SOCKETS)
				{
					user_list_t *user = find_user_from_fd(sock_set[i].fd);
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
				char buf[1024];
				int n = read(STDIN_FILENO, buf, sizeof(buf));
				buf[n] = '\0';
				log_line("Ricevuto: '%s', ma devo ancora implementare il resto...\n", buf);
				rewrite_prompt();
				// TODO
				break;

			case RESERVED_LISTENER:
				int ret = accept_user(ser_sock);
				if (ret == -1)
				{
					log_line("Utente rifiutato.\n");
					break;
				}

				user_list_t *last_user = (user_list_t *)user_list.prev;
				uint16_t port = ntohs(last_user->data.sockaddr.sin_port);
				char netaddr[20];
				inet_ntop(AF_INET, &last_user->data.sockaddr.sin_addr, netaddr, sizeof(netaddr));
				log_line("New User! (%d) %s:%u\n", current_users, netaddr, port);
				break;

			default:
				int command_id = -1;
				command_t *command = recv_command(sock_set[i].fd);
				if (!command)
				{
					fprintf(stderr, "Errore nella struttura del comando");
					user_list_t *user = find_user_from_fd(sock_set[i].fd);
					disconnect_user(user);
					break;
				}

				command_id = command->command;
				user_list_t *user = find_user_from_fd(fd);
				if (!user)
				{
					fprintf(stderr, "PANIC: non trovato l'utente associato al file descriptor %d", fd);
					end_printing();
					exit(1);
				}

				if (!network_handling_table[command_id])
				{
					fprintf(stderr, "non trovato l'handler associato al comando %s", str_command_tokens[command_id]);
				}
				else
				{
					network_handling_table[command_id](user, command);
				}

				destroy_command_list(command);
				free(command);
				break;
			}
		}
	}

	return 0;
}