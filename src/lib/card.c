#include "card.h"
#include <string.h>

static void filter_newline(char *str)
{
	char *read = str, *write = str;
	while (*read)
	{
		if (*read != '\n')
		{
			*write = *read;
			write++;
		}
		read++;
	}
	*write = '\0';
}

card_t *new_card(uint64_t ID, char *desc)
{
	card_t *card = malloc(sizeof(*card));
	if (!card)
		goto error;

	init_list(&card->list);
	card->ID = ID;
	card->last_changed = time(NULL);
	card->desc = NULL;
	card->user = 0;

	if (desc)
	{
		filter_newline(desc);
		size_t bufsize = (size_t)strlen(desc) + 1U;
		card->desc = malloc(bufsize);
		if (!card->desc)
			goto card_created_error;
		memcpy(card->desc, desc, bufsize);
	}

	return card;

card_created_error:
	free(card);
error:
	return NULL;
}

void destroy_card(card_t *card)
{
	if (!card)
		return;

	pop_elem(&card->list);

	if(card->desc)
		free(card->desc);

	free(card);
}

void clear_card_list(list_t *card_list)
{
	if (!card_list)
		return;

	while (!list_empty(card_list))
	{
		card_t *card = (card_t *)card_list->next;
		destroy_card(card);
	}
}