#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include <assert.h>
#include "parsing.h"
#include "list.h"
#include <arpa/inet.h>
#include <unistd.h>

struct test_list
{
	list_t list_elem;
	int info;
};

void print_list(list_t *b)
{
	if (list_empty(b))
	{
		return;
	}
	list_t *iter = b->next;
	while (iter != b)
	{
		int info = ((struct test_list *)iter)->info;
		printf("%d ", info);
		iter = iter->next;
	}
	printf("\n");
}

int main(int argc, char** argv)
{
	short port = 0;

	if(argc==2){
		port = atoi(argv[1]);
	}

	if(port == 0){
		port = 5679;
	}
	printf("%s\n%u\n", argv[1], port);

	int mysock = socket(AF_INET, SOCK_STREAM, 0);
	int one = 1;
	setsockopt(mysock, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(one));
	struct sockaddr_in myaddr;
	inet_pton(AF_INET, "127.0.0.1", &myaddr.sin_addr);
	myaddr.sin_family = AF_INET;
	myaddr.sin_port = htons(port);
	int er = bind(mysock, (struct sockaddr*)&myaddr, sizeof(myaddr));
	if (er == -1){
		fprintf(stderr, "Ohno");
		return 1;
	}

	struct sockaddr_in servaddr;
	inet_pton(AF_INET, "127.0.0.1", &servaddr.sin_addr);
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(5678);
	int conn = connect(mysock, (struct sockaddr*)&servaddr, sizeof(servaddr));

	char buf[] = "HELLO \n\n";
	send(mysock, buf, sizeof(buf), 0);

	char recbuf[100];
	int n = recv(mysock,recbuf, sizeof(recbuf)-1, 0);
	recbuf[n] = '\0';
	printf("%s\n",recbuf);

	sleep(2);
	close(conn);
	return 0;
}