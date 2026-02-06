#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include <assert.h>
#include <setjmp.h>
#include <signal.h>
#include <sys/poll.h>

#include "parsing.h"
#include "printing.h"
#include "list.h"
#include "utente-net.h"
#include "utente-cli.h"

struct test_list
{
	list_t list_elem;
	int info;
};

struct pollfd topoll[3];

jmp_buf exit_jump_buffer;
int running = 1;
void exit_handler(int sig)
{
	(void)(sig);
	running = 0;
}

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
	if(setjmp(exit_jump_buffer) == 1)
		goto end;
	signal(SIGINT, exit_handler);

	topoll[0] = (struct pollfd){.fd = STDIN_FILENO, .events = POLLIN};

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
		return 1;
	topoll[1] = (struct pollfd){.fd = mysock, .events = POLLIN};

	int linstener = init_listener_socket();
	if (linstener == -1)
		return 1;
	topoll[2] = (struct pollfd){.fd = linstener, .events = POLLIN};

	init_printing();

	while(running)
	{
		poll(topoll, 3, -1);
		if (topoll[0].revents & POLLIN)
		{
			cli_event();
			rewrite_prompt();
		}
		if (topoll[1].revents & POLLIN)
		{
			command_t *command = recv_command(mysock);
			if (command && network_handlers[command->id])
			{
				network_handlers[command->id](command);
			}
			destroy_command(command);
		}
		if (topoll[2].revents & POLLIN)
		{
			accept_request(linstener);
		}
	}


end:
	sendf(mysock, "%s\n\n", str_command_tokens[QUIT]);
	end_printing();
	return 0;
}