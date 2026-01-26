#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include <assert.h>
#include "parsing.h"
#include "list.h"
#include <malloc.h>


struct test_list {
	list_t list_elem;
	int info;
};

void print_list(list_t *b){
	if (list_empty(b)){
		return;
	}
	list_t *iter = b->next;
	while(iter != b){
		int info = ((struct test_list*)iter)->info;
		printf("%d ", info);
		iter = iter->next;
	}
	printf("\n");
}


int main()
{
	list_t head;
	init_list(&head);

	struct test_list a = {.info = 1};
	struct test_list b = {.info = 2};

	push_back(&head, (list_t*)&a);
	push_back(&head, (list_t*)&b);
	print_list(&head);
	pop_back(&head);
	print_list(&head);
	return 0;
}