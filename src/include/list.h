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

static inline list_t *pop_back(list_t *head)
{
	if (list_empty(head))
		return NULL;

	list_t *out = head->prev;
	list_t *prev = out->prev;
	head->prev = prev;
	prev->next = head;
	init_list(out);
	return out;
}

#endif