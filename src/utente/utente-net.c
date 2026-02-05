#include <sys/select.h>

#include "utente-net.h"
#include "parsing.h"

int init_socket(uint16_t port)
{
	port = htons(port);

	int mysock = socket(AF_INET, SOCK_STREAM, 0);
	if (mysock == -1)
		goto error;

	// Permetti di riusare la stessa porta dopo poco tempo
	int one = 1;
	int ret = setsockopt(mysock, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(one));
	if (ret == -1)
		goto socket_made_error;

	struct sockaddr_in myaddr;
	myaddr.sin_family = AF_INET;
	myaddr.sin_addr.s_addr = INADDR_ANY;
	myaddr.sin_port = port;

	ret = bind(mysock, (struct sockaddr *)&myaddr, sizeof(myaddr));
	if (ret == -1)
		goto socket_made_error;

	struct sockaddr_in servaddr;
	inet_pton(AF_INET, "127.0.0.1", &servaddr.sin_addr);
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(5678);

	ret = connect(mysock, (struct sockaddr *)&servaddr, sizeof(servaddr));
	if (ret == -1)
		goto socket_made_error;

	sendf(mysock, "%s", str_command_tokens[HELLO]);

	// Aspetta conferma dal server
	command_t *command = recv_command(mysock);
	if (!command)
		goto socket_made_error;
	command_token_t command_token = command->id;
	destroy_command(command);

	if (command_token != HELLO)
		goto socket_made_error;

	return mysock;

	// Gestione dell'errore
socket_made_error:
	close(mysock);
error:
	return -1;
}

int init_listener_socket(uint16_t port)
{

	int listener_sock = socket(AF_INET, SOCK_STREAM, 0);
	if (listener_sock == -1)
		goto error;

	int one = 1;
	int ret = setsockopt(listener_sock, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(one));
	if (ret == -1)
		goto socket_made_error;

	struct sockaddr_in myaddr;
	myaddr.sin_family = AF_INET;
	myaddr.sin_addr.s_addr = INADDR_ANY;
	myaddr.sin_port = port;

	ret = bind(listener_sock, (struct sockaddr *)&myaddr, sizeof(myaddr));
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

	command_t *command = recv_command(user_sock);
	if (!command)
		goto sock_created_error;

	if (command->id != REVIEW_CARD) // Interazione non valida
	{
		goto command_created_error;
	}

	if (strcmp(command->content, "request") == 0)
	{
		// TODO: Gestisci richiesta approvazione, spawna il thread...
	} else if (strcmp(command->content, "accept"))
	{
		// TODO: gestisci approvazione, registra risposta, controlla
		// che non siano rimasti altri utenti da cui ricevere la risposta...
	}else
	{
		fprintf(stderr, "Formato non valido per REVIEW_CARD");
	}

	close(listener_sock); // Connessione non persistente
	destroy_command(command);
	return 0;

command_created_error:
	destroy_command(command);

sock_created_error:
	close(listener_sock);

error:
	return -1;
}
