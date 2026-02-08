/**
 * @brief Interfaccia per una lista doppiamente concatenata:
 * una lista vuota è definita come una lista il cui puntatore
 * `next` e `prev` puntano a se stessa.
 * 
 * Esempio di utilizzo in una struttura dati generica:
 * 
 * struct esempio {
 *     list_t list;
 *     int data; 
 *     ...
 * };
 * 
 * Per iterare su una lista di `esempio`:
 * 
 * list_t *iter = head->next;
 * 
 * Per accedere alla struttura dati a partire da
 * un elemento della lista basta fare un cast:
 * 
 * esempio_t *esempio_iter = (esempio_t *)iter;
 * 
 * Questo è possibile perché `list` è il primo campo della
 * struttura `esempio` e quindi ha offset pari a 0 (System V ABI).
 * 
 * Esempio iterazione su lista `head`:
 * 
 * list_t *iter = head->next;
 * while(iter != head) {
 *     esempio_t *esempio_iter = (esempio_t *)iter;
 *     // Operazioni su esempio_iter
 *     iter = iter->next;
 * }
 * 
 * Per fermare l'iterazione basta controllare se `iter` è uguale a `head`: .
 * `head` è solo la testa della lista e non contiene dati.
 */
#ifndef LIST_H
#define LIST_H

#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>

typedef struct list
{
	struct list *next;
	struct list *prev;
} list_t;

/**
 * @brief Inizializza una lista vuota, portandola
 * in uno stato consistente
 */
static inline void init_list(list_t *head)
{
	head->next = head->prev = head;
}


/**
 * @brief Controlla se una lista è vuota
 * 
 * @returns `true` se la lista è vuota, `false` altrimenti
 */
static inline bool list_empty(list_t *head)
{
	return head->next == head;
}

/**
 * @brief Inserisce `elem` alla fine della lista `head`
 */
static inline void push_back(list_t *head, list_t *elem)
{
	list_t *prev = head->prev;

	elem->next = head;
	elem->prev = prev;

	head->prev = prev->next = elem;
}

/**
 * @brief Rimuove `elem` dalla lista a cui appartiene.
 * 
 * @returns `elem` se è stato rimosso, `NULL` se la lista è vuota
 */
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

/**
 * @brief Rimuove l'ultimo elemento della lista `head` e lo ritorna.
 * 
 * @returns l'ultimo elemento della lista se la lista non è vuota, `NULL` altrimenti
 */
static inline list_t *pop_back(list_t *head)
{
	return pop_elem(head->prev);
}

#endif