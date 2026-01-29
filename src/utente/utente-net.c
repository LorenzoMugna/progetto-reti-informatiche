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
	command_token_t command_token = command->command;
	destroy_command_list(command);
	free(command);

	if(command_token!=HELLO)
		goto socket_made_error;
	
	return mysock;

	// Gestione dell'errore
socket_made_error:
	close(mysock);
error:
	return -1;
}