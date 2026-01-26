#include "state-handler.h"

/* -------------- Definizione entità globali ------------------*/ 
/* --(visibili in tutte le unità di compilazione di lavagna) -- */

list_t to_do_list;
list_t doing_list;
list_t done_list;

list_t user_list;


void print_cardlist(list_t *b)
{
	if (!b || list_empty(b))
	{
		printf("\n");
		return;
	}

	list_t *iter = b->next;
	while (b != iter)
	{
		cardlist_t *cardp = (cardlist_t *)iter;
		card_t *curcard = &cardp->card;
		printf("\t%lu: %s\n", curcard->ID, curcard->mess);
		iter = iter->next;
	}

	printf("\n");
}

void show_lavagna_handler()
{
	printf("To Do:\n");
	print_cardlist(&to_do_list);
	printf("Doing:\n");
	print_cardlist(&doing_list);
	printf("Done:\n");
	print_cardlist(&done_list);
	printf("-----------------------------\n");
}

int main()
{
	init_list(&to_do_list);
	init_list(&doing_list);
	init_list(&done_list);
	char msg[] = "Hello world!";
	for (int i = 0; i < 3; i++)
	{
		cardlist_t *elem = malloc(sizeof(*elem));
		elem->card.ID = i + 1;
		elem->card.mess = msg;

		push_back(&to_do_list, &elem->list_elem);
	}
	show_lavagna_handler();

	list_t *elem = pop_back(&to_do_list);
	push_back(&done_list, elem);
	show_lavagna_handler();


	char testcmnd[] = "HANDLE_CARD arg1 aege3 ewor soerum ";
	char temp[strlen(testcmnd)+1];
	memcpy(temp, testcmnd, strlen(testcmnd)+1);
	// test memory leak
	command_t *test = parse_command(temp);
	destroy_command_list(test);
	free(test);

	memcpy(temp, testcmnd, strlen(testcmnd)+1);
	test = parse_command(temp);

	if (test){
		printf("%d: %s (%d arg)\n", test->command, str_command_tokens[test->command], test->argc);
	}else{
		printf("Not found.\n");
	}

	list_t *iter = test->param_list.next;
	while (iter != &test->param_list){
		command_arg_list_t *elem = (command_arg_list_t*) iter;
		printf("%s\n", elem->buffer);
		iter = iter->next;
	}

	destroy_command_list(test);
	free(test);
	return 0;
}