#include "state-handler.h"
#include "lavagna-net.h"

/* -------------- Definizione entità globali ------------------*/
/* --(visibili in tutte le unità di compilazione di lavagna) -- */

list_t to_do_list;
list_t doing_list;
list_t done_list;

list_t user_list;

user_list_t *user_table[MAX_PORT];

void print_cardlist(list_t *b)
{
	if (!b || list_empty(b))
	{
		printf("\n");
		return;
	}

	list_t *iter = b->next;
	while (b != iter)
	{
		card_list_t *cardp = (card_list_t *)iter;
		card_t *curcard = &cardp->card;
		printf("\t%lu: %s\n", curcard->ID, curcard->mess);
		iter = iter->next;
	}

	printf("\n");
}

void show_lavagna_handler()
{
	printf("To Do:\n");
	print_cardlist(&to_do_list);
	printf("Doing:\n");
	print_cardlist(&doing_list);
	printf("Done:\n");
	print_cardlist(&done_list);
	printf("-----------------------------\n");
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
		printf("%d: %s (%d arg)\n", test->command, str_command_tokens[test->command], test->argc);
	}
	else
	{
		printf("Not found.\n");
	}

	list_t *iter = test->param_list.next;
	while (iter != &test->param_list)
	{
		command_arg_list_t *elem = (command_arg_list_t *)iter;
		printf("%s\n", elem->buffer);
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

int main()
{
	// Initing server state variables
	init_list(&to_do_list);
	init_list(&doing_list);
	init_list(&done_list);
	init_list(&user_list);

	int ser_sock = init_server();
	while (1)
	{
		poll(sock_set, current_users + RESERVED_SOCK_SET_SOCKETS, -1);
		for (uint32_t i = 0; i < current_users + RESERVED_SOCK_SET_SOCKETS; i++)
		{
			if (!(sock_set[i].revents & POLLIN))
				continue;

			int fd = sock_set[i].fd;
			switch (i)
			{
			case RESERVED_STDIN:
				char buf[1024];
				int n = read(STDIN_FILENO, buf,sizeof(buf));
				buf[n] = '\0';
				printf("Ricevuto: '%s', ma devo ancora implementare il resto...\n", buf);
				// TODO
				break;

			case RESERVED_LISTENER:
				int ret = accept_user(ser_sock);
				if (ret == -1)
				{
					printf("Utente rifiutato.\n");
					break;
				}

				user_list_t *last_user = (user_list_t *)user_list.prev;
				uint16_t port = ntohs(last_user->data.sockaddr.sin_port);
				char netaddr[20];
				inet_ntop(AF_INET, &last_user->data.sockaddr.sin_addr, netaddr, sizeof(netaddr));
				printf("New User! (%d) %s:%u\n", current_users, netaddr, port);
				break;

			default:
				command_t *command = recv_command(sock_set[i].fd);
				if (!command){
					fprintf(stderr, "Errore nella struttura del comando.");
					break;
				}
				int command_id = command->command;

				user_list_t *user = find_user_from_fd(fd);
				if (!user)
				{
					fprintf(stderr, "PANIC: non trovato l'utente associato al file descriptor %d", fd);
					exit(1);
				}

				if(!command_handling_table[command_id])
				{
					fprintf(stderr, "non trovato l'handler associato al comando %s", str_command_tokens[command_id]);
				}
					
				command_handling_table[command_id](user, command);

				destroy_command_list(command);
				free(command);
				break;
			}
		}
	}
	return 0;
}