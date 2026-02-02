#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include <assert.h>
#include <sys/poll.h>

#include "parsing.h"
#include "printing.h"
#include "list.h"
#include "utente-net.h"

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

void poll_test()
{
	struct pollfd topoll;
	topoll.fd = STDIN_FILENO;
	topoll.events = POLLIN;

	while (1)
	{
		poll(&topoll, 1, -1);
		char buf[1024];
		read(STDIN_FILENO, buf, sizeof(buf));
		strtok(buf, "\n");
		printf("%s Mi bomboclut!\n", buf);
	}
}

void print_test()
{
	init_printing();
	char buf[256];
	while (fgets(buf, 256, stdin))
	{
		log_line(buf);
		rewrite_prompt();
		log_line("test logging\n\n");
	}
	end_printing();
	return;
}
int main(int argc, char **argv)
{

	// print_test();
	// return 0;
	short port = 0;

	if (argc == 2)
	{
		port = atoi(argv[1]);
	}

	if (port == 0)
	{
		port = 5679;
	}
	printf("%s\n%u\n", argv[1], port);

	int mysock = init_socket(port);
	if (mysock == -1)
	{
		return 1;
	}

	int linstener = init_listener_socket(port);
	if (linstener == -1)
	{
		return 1;
	}

	char bud[256];
	init_printing();
	while(fgets(bud, sizeof(bud), stdin))
	{
		log_line("%s", bud);
		sendf(mysock, "%s", bud);
		rewrite_prompt();
	}

	end_printing();
	sendf(mysock, "%s\n\n", str_command_tokens[QUIT]);

	return 0;
}