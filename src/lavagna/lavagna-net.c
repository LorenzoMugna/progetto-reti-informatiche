#include "lavagna-net.h"
#include "state-handler.h"
#include "printing.h"

#include <stdio.h>
#include <unistd.h>

struct pollfd sock_set[RESERVED_SOCK_SET_SOCKETS + MAX_USERS];
uint32_t current_users;

void remove_from_sock_set(int fd)
{
	uint32_t w = RESERVED_SOCK_SET_SOCKETS,
			 r = RESERVED_SOCK_SET_SOCKETS;
	while (r < current_users + RESERVED_SOCK_SET_SOCKETS)
	{
		if (sock_set[r].fd == fd)
			r++;
		if (r != w)
		{
			sock_set[w] = sock_set[r];
		}
		r++;
		w++;
	}
	current_users--;
	memset(&sock_set[current_users + RESERVED_SOCK_SET_SOCKETS], 0, sizeof(sock_set[0]));
}

user_t *find_user_from_fd(int fd)
{
	// Scorri la lista degli utenti e trovane uno con fd che corrisponda
	list_t *iter = user_list.next;
	while (iter != &user_list)
	{
		user_t *cast_iter = (user_t *)iter;
		if (cast_iter->socket == fd)
			return cast_iter;

		iter = iter->next;
	}
	return NULL;
}

int init_server()
{
	// Inizializzazzione variabili di stato
	current_users = 0;
	memset(sock_set, 0, sizeof(sock_set));

	int listener_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (listener_fd == -1)
		goto socket_error;

	int one_ptr = 1;
	setsockopt(listener_fd, SOL_SOCKET, SO_REUSEADDR, &one_ptr, sizeof(one_ptr));

	struct sockaddr_in listen_addr;
	listen_addr.sin_family = AF_INET;
	listen_addr.sin_addr.s_addr = INADDR_ANY;
	listen_addr.sin_port = htons(LAVAGNA_PORT);

	if (bind(listener_fd, (struct sockaddr *)&listen_addr, sizeof(listen_addr)) == -1)
	{
		goto bind_error;
	}

	if (listen(listener_fd, CONNECTION_BACKLOG) == -1)
	{
		goto listen_error;
	}

	sock_set[RESERVED_LISTENER] = (struct pollfd){.fd = listener_fd, .events = POLLIN};
	sock_set[RESERVED_STDIN] = (struct pollfd){.fd = STDIN_FILENO, .events = POLLIN};
	return listener_fd;

	// Gestione degli errori
listen_error:
bind_error:
	close(listener_fd);
socket_error:
	return -1;
}

int accept_user(int server_fd)
{
	// Accetta connessione TCP
	struct sockaddr_in useraddr;
	socklen_t size_useraddr = sizeof(useraddr);
	int user_sock = accept(server_fd, (struct sockaddr *)&useraddr, &size_useraddr);
	if (user_sock == -1)
		goto error;

	// Attendi un comando con timeout
	fd_set user_fd_set;
	FD_ZERO(&user_fd_set);
	FD_SET(user_sock, &user_fd_set);
	struct timeval timeout = _LAVAGNA_TIMEOUT;
	int select_return = select(user_sock + 1, &user_fd_set, NULL, NULL, &timeout);

	// Errore select o utente non ha risposto in tempo
	if (select_return == -1 || !FD_ISSET(user_sock, &user_fd_set))
		goto sock_created_error;

	// Parsa comando e controlla che sia un HELLO
	command_t *command = recv_command(user_sock);
	if (!command)
		goto sock_created_error;
	command_token_t command_id = command->id;
	destroy_command(command);

	if (command_id != HELLO)
		goto sock_created_error;

	if (current_users >= MAX_USERS)
		goto connection_rejected_error;

	uint16_t user_port = ntohs(useraddr.sin_port);
	if (user_table[user_port] != NULL)
		goto connection_rejected_error;

	// Creazione nuova entry nella user_list e user_table
	user_t *newuser = new_user(&useraddr, user_sock);
	if (!newuser)
		goto connection_rejected_error;

	push_back(&user_list, &newuser->list);
	user_table[user_port] = newuser;

	// Aggiunta del socket al sock_set
	sock_set[current_users + RESERVED_SOCK_SET_SOCKETS] = (struct pollfd){
		.fd = user_sock,
		.events = POLLIN | POLLERR};
	current_users++;

	// Conferma la connessione al client inviandogli HELLO
	sendf(user_sock, "%s", str_command_tokens[HELLO]);

	// TODO: assegna nuova carta
	return user_sock;

	// Gestione errori
connection_rejected_error:
	// Rifiuta il client inviandogli QUIT
	sendf(user_sock, "%s", str_command_tokens[QUIT]);
sock_created_error:
	close(user_sock);
error:
	return -1;
}

int disconnect_user(user_t *user)
{
	// Passi per disconnesione utente
	// 1. sposta eventuale carta da doing a To Do
	card_t *card = user->handled_card;
	if (card)
	{
		pop_elem(&card->list);
		push_back(&to_do_list, &card->list);
		card->user = 0;
	}

	uint16_t user_port = ntohs(user->sockaddr.sin_port);
	int user_socket = user->socket;
	char netaddr[20];
	inet_ntop(AF_INET, &user->sockaddr.sin_addr, netaddr, sizeof(netaddr));

	// Invia comando di QUIT all'utente. Ignora errori
	// nella ricezione e continua con il processo di
	// disconnessione (caso d'esempio: l'utente si è
	// già disconnesso quando questa funzione va in esecuzione)
	int err = sendf(user_socket, "%s", str_command_tokens[QUIT]);

	// 2. rimuovi user dalla lista di utenti
	memset(&user_table[user_port], 0, sizeof(user_table[user_port]));

	// 3. rimuovi fd dalla sock_set
	remove_from_sock_set(user_socket);

	destroy_user(user);

	log_line("Utente %s:%d disconnesso\n", netaddr, user_port);
	return err; // ritorna lo stato della sendf
}

/* ---- HANDLER EVENTI RICEVUTI ---- */
int ignore_command(user_t *user, command_t *command)
{
	(void)(user);
	(void)(command);

	return 0;
}

int handle_QUIT(user_t *user, command_t *command)
{
	if (!user || !command)
		return -1;

	disconnect_user(user);
	return 0;
}

int handle_CREATE_CARD(user_t *user, command_t *command)
{
	(void)(user);

	if (!command)
		goto error;

	card_t *card = new_card(last_card_id + 1, command->content);
	if (!card)
		goto error;

	push_back(&to_do_list, &card->list);
	last_card_id++;

	show_lavagna_handler();
	// TODO: assegna la carta appena creata
	return 0;
error:
	return -1;
}

int handle_SHOW_LAVAGNA(user_t *user, command_t *command)
{
	(void)command;

	if (!user)
		goto error;

	int user_socket_fd = user->socket;

	char a[1024];
	build_lavagna(a, sizeof(a));
	int err = sendf(user_socket_fd, "%s %s", str_command_tokens[SHOW_LAVAGNA], a);
	if (err == -1)
		goto error;

	return 0;

error:
	return -1;
}

int handle_PONG_LAVAGNA(user_t *user, command_t *command)
{
	(void)command;
	if (!user)
		goto error;

	// Reagisci ad un PONG_LAVAGNA reimpostando un timeout PING_UTENTE
	// ma solo se si stava effettivamente aspettando un PONG_LAVAGNA
	if (user->timeout_type != TIMEOUT_PONG_LAVAGNA)
		return 0;
	
	user->timeout_type = TIMEOUT_PING_UTENTE;
	user->next_timeout = time(NULL) + PING_TIMEOUT;

error:
	return -1;
}

int handle_REQUEST_USER_LIST(user_t *user, command_t *command)
{
	(void)command;

	if (!user)
		goto error;

	int user_socket_fd = user->socket;

	char buf[1024];
	build_user_list(buf, sizeof(buf));
	int err = sendf(user_socket_fd, "%s %s", str_command_tokens[SEND_USER_LIST], buf);
	if (err == -1)
		goto error;
	return 0;

error:
	return -1;
}

int handle_CARD_DONE(user_t *user, command_t *command)
{
	(void)command;

	if (!user)
		goto error;

	if (!user->handled_card)
		goto error;

	card_t *card = user->handled_card;
	pop_elem(&card->list);
	card->last_changed = time(NULL);
	push_back(&done_list, &card->list);

	user->handled_card = NULL;

	// Cancella il timeout: non sta più gestendo carte
	user->timeout_type = TIMEOUT_NONE;
	user->next_timeout = 0;

	// TODO: assegna nuova carta
error:
	return -1;
}

command_handler_t network_handling_table[N_COMMAND_TOKENS] = {
	[HELLO] = ignore_command, // Gestito nella routine di accettazione
	[QUIT] = handle_QUIT,
	[CREATE_CARD] = handle_CREATE_CARD,
	[MOVE_CARD] = ignore_command, // Un utente non richiede direttamente lo spostamento
	[SHOW_LAVAGNA] = handle_SHOW_LAVAGNA,
	[SEND_USER_LIST] = ignore_command, // Un utente deve inviare REQUEST_USER_LIST
	[PING_USER] = ignore_command,	   // Non previsto da un utente
	[PONG_LAVAGNA] = handle_PONG_LAVAGNA,
	[HANDLE_CARD] = ignore_command, // Non previsto da un utente
	[ACK_CARD] = NULL,
	[REQUEST_USER_LIST] = handle_REQUEST_USER_LIST,
	[REVIEW_CARD] = ignore_command, // Riservato agli utenti
	[CARD_DONE] = handle_CARD_DONE,
};