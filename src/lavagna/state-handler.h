#ifndef STATE_HANDLER_H
#define STATE_HANDLER_H

#include <arpa/inet.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include "card.h"
#include "list.h"
#include "parsing.h"

/* ---------- Entità globali (visibili in tutte le unità di compilazione di lavagna) -----------------*/
extern list_t to_do_list; // Contenitore di `cardlist_t`
extern list_t doing_list; // Contenitore di `cardlist_t`
extern list_t done_list;  // Contenitore di `cardlist_t`
extern list_t user_list;  // Contenitore di `user_list_t`

typedef struct user
{
	struct sockaddr_in sockaddr; // Indirizzo dell'utente
	int socket;					 // FD del socket associato all'utente
	bool handling_card;			 // se l'utente sta gestendo una carta o meno
	card_t *handled_card;		 // la carta che l'utente sta gestendo

} user_t;

typedef struct user_list
{
	list_t list; // Puntatori per creare la lista
	user_t data; // Dati dell'utente
}user_list_t;
/**
 * @brief stampa una lista di card, evidenziando ID, e descrizione dell'attività
 * @param b puntatore alla testa della cardlist
 */
void print_cardlist(list_t *b);

/**
 * @brief funzione che implementa il comando `SHOW_LAVAGNA`: le tre colonne sono stampate una dopo l'altra
 */
void show_lavagna_handler();

/**
 * @brief esegue un comando ricevuto da terminale, già parsato.
 * @param command stringa ricevuta da terminale
 */
void command_dispatch(char *command);

#endif