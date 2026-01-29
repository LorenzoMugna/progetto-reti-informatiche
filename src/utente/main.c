#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include <assert.h>

#include "parsing.h"
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

	int mysock = init_socket(port);
	if(mysock==-1)
	{
		return 1;
	}
	sleep(2);

	sendf(mysock, "%s\n\n", str_command_tokens[QUIT]);
	return 0;
}