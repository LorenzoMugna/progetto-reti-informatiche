#include <sys/select.h>
#include <signal.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>

#include "utente-net.h"
#include "card.h"
#include "parsing.h"
#include "printing.h"

int my_socket;
struct sockaddr_in my_address;

user_state_t current_user_state;
card_t *handled_card;

list_t missing_reviews;

useraddr_t *new_useraddr()
{
	useraddr_t *useraddr = malloc(sizeof(useraddr_t));
	if (!useraddr)
		goto error;

	init_list(&useraddr->list);
	return useraddr;
error:
	return NULL;
}

void destroy_useraddr(useraddr_t *useraddr)
{
	if (!useraddr)
		return;

	pop_elem(&useraddr->list);
	free(useraddr);
}

void clear_useraddr_list(list_t *useraddr_list)
{
	while (!list_empty(useraddr_list))
	{
		useraddr_t *useraddr = (useraddr_t *)useraddr_list->next;
		destroy_useraddr(useraddr);
	}
}

int init_socket(uint16_t port)
{
	port = htons(port);
	init_list(&missing_reviews);
	current_user_state = STATE_IDLE;
	handled_card = NULL;

	int mysock = socket(AF_INET, SOCK_STREAM, 0);
	if (mysock == -1)
		goto error;

	// Permetti di riusare la stessa porta dopo poco tempo
	int ret = setsockopt(mysock, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int));
		setsockopt(mysock, SOL_SOCKET, SO_REUSEPORT, &(int){1}, sizeof(int));
	if (ret == -1)
		goto socket_made_error;

	my_address.sin_family = AF_INET;
	my_address.sin_addr.s_addr = INADDR_ANY;
	my_address.sin_port = port;

	ret = bind(mysock, (struct sockaddr *)&my_address, sizeof(my_address));
	if (ret == -1)
		goto socket_made_error;

	struct sockaddr_in server_addr;
	inet_pton(AF_INET, "127.0.0.1", &server_addr.sin_addr);
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(5678);

	ret = connect(mysock, (struct sockaddr *)&server_addr, sizeof(server_addr));
	if (ret == -1)
		goto socket_made_error;

	ret = sendf(mysock, "%s", str_command_tokens[HELLO]);
	if (ret == -1)
		goto socket_made_error;

	// Aspetta conferma dal server
	command_t *command;
	recv_command(mysock, &command);
	if (!command)
		goto socket_made_error;
	command_token_t command_token = command->id;
	destroy_command(command);

	if (command_token != HELLO)
		goto socket_made_error;

	my_socket = mysock;
	return mysock;

	// Gestione dell'errore
socket_made_error:
	close(mysock);
error:
	return -1;
}

int init_listener_socket()
{

	int listener_sock = socket(AF_INET, SOCK_STREAM, 0);
	if (listener_sock == -1)
		goto error;

	int one = 1;
	int ret = setsockopt(listener_sock, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(one));
		setsockopt(listener_sock, SOL_SOCKET, SO_REUSEPORT, &(int){1}, sizeof(int));
	if (ret == -1)
		goto socket_made_error;

	ret = bind(listener_sock, (struct sockaddr *)&my_address, sizeof(my_address));
	if (ret == -1)
		goto socket_made_error;

	ret = listen(listener_sock, MAX_BACKLOG);
	if (ret == -1)
		goto socket_made_error;

	return listener_sock;

socket_made_error:
	close(listener_sock);
error:
	return -1;
}

void *review_thread_func(void *arg)
{
	int secs = 1;
	sleep(secs);
	struct sockaddr_in *useraddr = (struct sockaddr_in *)arg;
	int newsock = socket(AF_INET, SOCK_STREAM, 0);
	setsockopt(newsock, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int));
	setsockopt(newsock, SOL_SOCKET, SO_REUSEPORT, &(int){1}, sizeof(int));
	int ret = bind(newsock, (struct sockaddr *)&my_address, sizeof(my_address));
	if (ret == -1)
	{
		perror("thread bind");
		exit(1);
	}
	ret = connect(newsock, (struct sockaddr *)useraddr, sizeof(*useraddr));
	if (ret == -1)
	{
		perror("thread connect");
		exit(1);
	}
	while (
		sendf(newsock, "%s accept",
			  str_command_tokens[REVIEW_CARD]) == -1)
		;
	free(arg);
	close(newsock);
	return NULL;
}

int accept_request(int listener_sock)
{
	// Accetta connessione TCP
	struct sockaddr_in useraddr;
	socklen_t size_useraddr = sizeof(useraddr);
	int user_sock = accept(listener_sock, (struct sockaddr *)&useraddr, &size_useraddr);
	if (user_sock == -1)
	{
		goto error;
	}

	// Attendi un comando con timeout
	fd_set user_fd_set;
	FD_ZERO(&user_fd_set);
	FD_SET(user_sock, &user_fd_set);
	struct timeval timeout = _UTENTE_TIMEOUT;
	int select_return = select(user_sock + 1, &user_fd_set, NULL, NULL, &timeout);

	// Errore select o utente non ha risposto in tempo
	if (select_return == -1 || !FD_ISSET(user_sock, &user_fd_set))
	{
		goto sock_created_error;
	}

	command_t *command;
	recv_command(user_sock, &command);

	if (!command)
		goto sock_created_error;


	if (command->id != REVIEW_CARD) // Interazione non valida
		goto command_created_error;

	char *tok = strtok(command->content, " ");
	if (!tok)
		goto command_created_error;

	if (strcmp(command->content, "request") == 0)
	{
		pthread_t review_thread;
		struct sockaddr_in *useraddr_copy = malloc(sizeof(struct sockaddr_in));
		memcpy(useraddr_copy, &useraddr, sizeof(struct sockaddr_in));
		pthread_create(&review_thread, NULL, review_thread_func, useraddr_copy);
		pthread_detach(review_thread);

	}
	else if (strcmp(command->content, "accept") == 0)
	{
		if (current_user_state != STATE_REVIEWING)
			goto command_created_error;
		// Rimuovi utente dalla lista di review mancanti
		for (list_t *it = missing_reviews.next; it != &missing_reviews; it = it->next)
		{
			useraddr_t *it_useraddr = (useraddr_t *)it;	
			if (it_useraddr->user_address.sin_port == useraddr.sin_port)
			{
				pop_elem(&it_useraddr->list);
				destroy_useraddr(it_useraddr);
				break;
			}
		}
		if (list_empty(&missing_reviews))
		{
			current_user_state = STATE_DONE;
			log_line("Tutte le review ricevute, puoi mandare CARD_DONE\n");
		}
	}
	else
	{
		fprintf(stderr, "Formato non valido per REVIEW_CARD");
	}

	//Manda in automatico? boh
	close(user_sock); // Connessione non persistente
	destroy_command(command);
	return 0;

command_created_error:
	destroy_command(command);

sock_created_error:
	close(listener_sock);

error:
	return -1;
}
/* ---- USER NETWORK HANDLERS ----*/

int ignore_command(command_t *command)
{
	(void)command;
	return 0;
}

int handle_QUIT(command_t *command)
{
	(void)command;

	// Termina semplicemente l'esecuzione.
	// La lavagna gestisce lo stato.
	log_line("Ricevuto QUIT dalla lavagna, termino l'esecuzione...\n");
	clear_useraddr_list(&missing_reviews);
	pid_t pid = getpid();
	kill(pid, SIGINT);
	return 0;
}

int handle_SHOW_LAVAGNA(command_t *command)
{
	// if (expected_from_lavagna != SHOW_LAVAGNA)
	// 	goto error;

	if (!command)
		goto error;

	log_line(command->content);
	return 0;
error:
	return -1;
}

int handle_SEND_USER_LIST(command_t *command)
{
	if (!command || !command->content)
		goto error;

	if (current_user_state != STATE_GETTING_USER_LIST)
		goto error;

	char *tok_state = NULL;
	char *tok = __strtok_r(command->content, " ", &tok_state);
	uint32_t n_users = atoi(tok);
	log_line("Lista utenti ricevuta (%d):\n", n_users);
	for (uint32_t i = 0; i < n_users - 1; i++)
	{
		tok = __strtok_r(NULL, " ", &tok_state);
		if (!tok)
			goto error;

		log_line("%s\n", tok);
		useraddr_t *useraddr = new_useraddr();
		if (!useraddr)
			goto error;

		parse_address(&useraddr->user_address, tok);
		push_back(&missing_reviews, &useraddr->list);

		if (useraddr->user_address.sin_addr.s_addr == INADDR_NONE)
		{
			log_line("Errore nel formato dell'indirizzo\n");
			clear_useraddr_list(&missing_reviews);
			goto error;
		}
	}
	log_line("\n");

	current_user_state = STATE_REVIEWING;
	for (list_t *it = missing_reviews.next; it != &missing_reviews; it = it->next)
	{
		// TODO: meno pigro
		useraddr_t *useraddr = (useraddr_t *)it;

		int newsock = socket(AF_INET, SOCK_STREAM, 0);
		setsockopt(newsock, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int));
		setsockopt(newsock, SOL_SOCKET, SO_REUSEPORT, &(int){1}, sizeof(int));

		int err = bind(newsock, (struct sockaddr *)&my_address, sizeof(my_address));
		err = connect(newsock, (struct sockaddr *)&useraddr->user_address, sizeof(useraddr->user_address));
		if (err == -1)
		{
			end_printing();
			perror("bind");
			exit(1);
			goto error;
		}
		while (
			sendf(newsock, "%s request",
				  str_command_tokens[REVIEW_CARD]) == -1)
			;
	
		close(newsock);
		
	}
	return 0;

error:
	log_line("Errore nella gestione della lista utenti\n");
	return -1;
}

int handle_PING_USER(command_t *command)
{
	(void)command;

	int err = sendf(my_socket, "%s ", str_command_tokens[PONG_LAVAGNA]);
	if (err == -1)
		goto error;
	return 0;

error:
	return -1;
}

int handle_HANDLE_CARD(command_t *command)
{
	if (!command || !command->content)
		goto error;

	// Rifiuta una carta se ne sto gestendo un'altra
	if (current_user_state != STATE_IDLE)
		goto error;

	// Parsa contenuto del comando
	//  1. Numero di utenti
	char *tok_state = NULL;
	char *tok = __strtok_r(command->content, " ", &tok_state);
	uint32_t n_users = atoi(tok);

	// 2. porte degli utenti
	for (uint32_t i = 0; i < n_users - 1; i++)
	{
		tok = __strtok_r(NULL, " ", &tok_state);

		// Comando malformato (e.g. numero avvisto di utenti troppo alto)
		if (!tok)
			goto error;

		// Scarta lista utenti; tanto al momento
		// della review viene richiesta nuovamente ¯\_(ツ)_/¯
	}
	// Comando malformato (e.g. numero avvisato di utenti troppo alto)
	if (!tok_state)
		goto error;

	int err = sendf(my_socket, "%s ", str_command_tokens[ACK_CARD]);
	if (err == -1)
		goto error;

	log_line("Carta ricevuta: %s\n", tok_state);
	current_user_state = STATE_HANDLING;
	handled_card = new_card(0, tok_state);
	return 0;

error:
	return -1;
}

network_handler_t network_handlers[N_COMMAND_TOKENS] = {
	[HELLO] = ignore_command,
	[QUIT] = handle_QUIT,
	[CREATE_CARD] = ignore_command,
	[MOVE_CARD] = ignore_command,
	[SHOW_LAVAGNA] = handle_SHOW_LAVAGNA,
	[SEND_USER_LIST] = handle_SEND_USER_LIST,
	[PING_USER] = handle_PING_USER,
	[PONG_LAVAGNA] = ignore_command,
	[HANDLE_CARD] = handle_HANDLE_CARD,
	[ACK_CARD] = ignore_command,
	[REQUEST_USER_LIST] = ignore_command,
	[REVIEW_CARD] = ignore_command,
	[CARD_DONE] = ignore_command};