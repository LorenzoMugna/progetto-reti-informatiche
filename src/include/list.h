/**
 * @author Lorenzo Mugnaioli
 * @brief definitions for list operations
 */
#ifndef LIST_H
#define LIST_H

#include <stdint.h>
#include <stdlib.h>

typedef struct list
{
	struct list *next;
	struct list *prev;
} list_t;

static inline void init_list(list_t *head)
{
	head->next = head->prev = head;
}

static inline int list_empty(list_t *head)
{
	return head->next == head;
}

static inline void push_back(list_t *head, list_t *elem)
{
	list_t *prev = head->prev;

	elem->next = head;
	elem->prev = prev;

	head->prev = prev->next = elem;
}


static inline list_t *pop_elem(list_t *elem)
{
	if (list_empty(elem))
		return NULL;

	list_t *prev = elem->prev;
	list_t *next = elem->next;
	next->prev = prev;
	prev->next = next;
	init_list(elem);
	return elem;
}

static inline list_t *pop_back(list_t *head)
{
	return pop_elem(head->prev);
}

#endif