#include "lavagna-net.h"
#include "unistd.h"
#include "state-handler.h"
#include <stdio.h>

struct pollfd sock_set[MAX_USERS];
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

user_list_t *find_user_from_fd(int fd)
{
	// Scorri la lista degli utenti e trovane uno con fd che corrisponda
	list_t *iter = user_list.next;
	while (iter != &user_list)
	{
		user_list_t *cast_iter = (user_list_t *)iter;
		if (cast_iter->data.socket == fd)
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
	sock_set[RESERVED_STDIN] = (struct pollfd){.fd = STDIN_FILENO, .events = POLLIN};

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
	{
		goto error;
	}

	// Attendi un comando con timeout
	fd_set user_fd_set;
	FD_ZERO(&user_fd_set);
	FD_SET(user_sock, &user_fd_set);
	struct timeval timeout = _LAVAGNA_TIMEOUT;
	int select_return = select(user_sock + 1, &user_fd_set, NULL, NULL, &timeout);

	// Errore select o utente non ha risposto in tempo
	if (select_return == -1 || !FD_ISSET(user_sock, &user_fd_set))
	{
		goto sock_created_error;
	}

	// Parsa comando e controlla che sia un HELLO
	command_t *command = recv_command(user_sock);
	if (!command)
		goto sock_created_error;
	command_token_t command_id = command->command;
	destroy_command_list(command);
	free(command);

	if (command_id != HELLO)
		goto sock_created_error;

	// Ottieni la porta e controlla che non sia già presente
	uint16_t user_port = ntohs(useraddr.sin_port);
	if (user_table[user_port] != NULL)
	{
		goto connection_rejected_error;
	}

	// Creazione nuova entry nella user_list e user_table
	user_list_t *newuser = malloc(sizeof(*newuser));
	if (!newuser)
		goto connection_rejected_error;
	memset(newuser, 0, sizeof(*newuser));
	memcpy(&newuser->data.sockaddr, &useraddr, size_useraddr);
	newuser->data.handling_card = false;
	newuser->data.socket = user_sock;

	push_back(&user_list, (list_t *)newuser);
	user_table[user_port] = newuser;

	// Voglio controllare solo in ricezione
	sock_set[current_users + RESERVED_SOCK_SET_SOCKETS] = (struct pollfd){.fd = user_sock, .events = POLLIN};
	current_users++;

	// Conferma la connessione al client inviandogli HELLO
	sendf(user_sock, "%s", str_command_tokens[HELLO]);
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

void disconnect_user(uint16_t port)
{
	port = 0;
	port = port;
	// TOUSE
}

/* ---- HANDLER EVENTI RICEVUTI ---- */
int handle_QUIT(user_list_t *user, command_t *command)
{
	printf("Gestendo disconnessione...\n");

	if (!user || !command)
		return -1;

	// Passi per disconnesione utente
	// 1. sposta eventuale carta da doing a To Do
	card_list_t *card = user->data.handled_card;
	if (card)
	{
		pop_elem((list_t *)card);
		push_back(&to_do_list, (list_t *)card);
	}

	uint16_t port = ntohs(user->data.sockaddr.sin_port);
	int fd = user->data.socket;

	// 2. rimuovi user dalla lista di utenti
	pop_elem((list_t *)user);
	memset(&user_table[port], 0, sizeof(user_table[port]));
	// 3. rimuovi fd dalla sock_set
	remove_from_sock_set(fd);
	close(fd);

	free(user);
	return 0;
}

command_handler_t command_handling_table[N_COMMAND_TOKENS] = {
	[QUIT] = handle_QUIT};