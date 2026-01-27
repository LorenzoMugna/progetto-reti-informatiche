#include "lavagna-net.h"
#include "unistd.h"
#include "state-handler.h"

int init_server_socket(){

	int out = socket(AF_INET, SOCK_STREAM, 0);
	if (out == -1){
		goto socket_error;
	}
	int one_ptr = 1;
	setsockopt(out, SOL_SOCKET, SO_REUSEADDR, &one_ptr, sizeof(one_ptr));

	struct sockaddr_in listen_addr;
	listen_addr.sin_family = AF_INET;
	listen_addr.sin_addr.s_addr = INADDR_ANY;
	listen_addr.sin_port = htons(LAVAGNA_PORT);
	
	if(bind(out, (struct sockaddr*)&listen_addr, sizeof(listen_addr))==-1){
		goto bind_error;
	}

	if (listen(out, CONNECTION_BACKLOG) == -1){
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


int accept_user(int server_fd){
	struct sockaddr_in useraddr;
	socklen_t size_useraddr = sizeof(useraddr);
	int user_sock = accept(server_fd, (struct sockaddr*)&useraddr, &size_useraddr);
	if (user_sock == -1){
		return -1;
	}

	//TODO: attendi ricezione HELLO con timeout

	user_list_t *newuser = malloc(sizeof(*newuser));

	memcpy(&newuser->data.sockaddr, &useraddr, size_useraddr);
	newuser->data.handling_card = false;
	newuser->data.socket = user_sock;

	push_back(&user_list, (list_t*)newuser);

	return user_sock;
}
