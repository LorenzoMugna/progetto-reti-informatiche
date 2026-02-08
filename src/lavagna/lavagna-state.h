/**
 * @brief Interfaccia per la gestione dello stato della lavagna
 * (lista utenti e lista di carte)
 */
#ifndef STATE_HANDLER_H
#define STATE_HANDLER_H

#include <arpa/inet.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#include "card.h"
#include "list.h"
#include "parsing.h"
#include "lavagna-timeout.h"
#include "user.h"

#define MAX_PORT ((1 << 16) - 1)

/* ---- VARIABILI DI STATO ---- */
extern list_t to_do_list; // Contenitore di `card_t`
extern list_t doing_list; // Contenitore di `card_t`
extern list_t done_list;  // Contenitore di `card_t`
extern list_t user_list;  // Contenitore di `user_t`

// Accesso rapido al descrittore di un utente con una porta specifica
extern user_t *user_table[MAX_PORT];

/**
 * @brief Inizializza le variabili di stato
 */
void init_state();

/**
 * @brief stampa una lista di card, evidenziando ID, e descrizione dell'attività
 * @param b puntatore alla testa della card
 */
void print_cardlist(list_t *b);


/**
 * @brief esegue un comando ricevuto da terminale, già parsato.
 * @param command stringa ricevuta da terminale
 */
void command_dispatch(char *command);

#endif