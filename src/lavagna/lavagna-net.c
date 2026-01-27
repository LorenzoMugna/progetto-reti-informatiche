#include "lavagna-net.h"
#include "unistd.h"
#include "state-handler.h"
#include <stdio.h>

char netbuffer[NETBUFFER_SIZE + 1];

int init_server_socket()
{
	int out = socket(AF_INET, SOCK_STREAM, 0);
	if (out == -1)
		goto socket_error;

	int one_ptr = 1;
	setsockopt(out, SOL_SOCKET, SO_REUSEADDR, &one_ptr, sizeof(one_ptr));

	struct sockaddr_in listen_addr;
	listen_addr.sin_family = AF_INET;
	listen_addr.sin_addr.s_addr = INADDR_ANY;
	listen_addr.sin_port = htons(LAVAGNA_PORT);

	if (bind(out, (struct sockaddr *)&listen_addr, sizeof(listen_addr)) == -1)
	{
		goto bind_error;
	}

	if (listen(out, CONNECTION_BACKLOG) == -1)
	{
		goto listen_error;
	}

	return out;

	// Gestione degli errori
listen_error:
bind_error:
	close(out);
socket_error:
	return -1;
}

int accept_user(int server_fd)
{
	struct sockaddr_in useraddr;
	socklen_t size_useraddr = sizeof(useraddr);
	int user_sock = accept(server_fd, (struct sockaddr *)&useraddr, &size_useraddr);
	if (user_sock == -1)
	{
		goto error;
	}

	uint16_t user_port = ntohs(useraddr.sin_port);
	if (user_table[user_port] != NULL)
	{
		goto sock_created_error;
	}

	fd_set user_fd_set;
	FD_ZERO(&user_fd_set);
	FD_SET(user_sock, &user_fd_set);
	struct timeval timeout = _LAVAGNA_TIMEOUT;

	int select_return = select(user_sock + 1, &user_fd_set, NULL, NULL, &timeout);
	if (select_return == -1 || !FD_ISSET(user_sock, &user_fd_set))
	{ // Errore select o utente non ha risposto in tempo
		goto sock_created_error;
	}

	recv(user_sock, netbuffer, NETBUFFER_SIZE, 0);
	netbuffer[NETBUFFER_SIZE] = '\0';
	command_t *command = parse_command(netbuffer);
	if (!command)
		goto sock_created_error;

	if (command->command != HELLO)
		goto command_parsed_error;

	free(command);

	// Comunica dimensione buffer al client
	char sendbuf[30];
	snprintf(sendbuf, sizeof(sendbuf), "HELLO %d\n", NETBUFFER_SIZE);
	send(user_sock, sendbuf, strlen(sendbuf+1), 0);

	// Creazione nuova entry nella user_list e user_table
	user_list_t *newuser = malloc(sizeof(*newuser));
	memcpy(&newuser->data.sockaddr, &useraddr, size_useraddr);
	newuser->data.handling_card = false;
	newuser->data.socket = user_sock;

	push_back(&user_list, (list_t *)newuser);
	user_table[user_port] = newuser;

	return user_sock;

	// Gestione errori
command_parsed_error:
	free(command);
sock_created_error:
	close(user_sock);
error:
	return -1;
}

void disconnect_user(uint16_t port)
{
	return;
	//TODO
	user_list_t *user_elem = user_table[port];
	if (!user_elem)
		return;

	int fd_user = user_elem->data.socket;
	char to_send[] = "QUIT\n";
	send(fd_user, to_send, sizeof(to_send),0);
	close(fd_user);
	pop_elem((list_t*)user_elem);
	free(user_elem);
}