/**
 * Funzioni e definizioni di tipi atte a gestire lo stato della
 * lavagna (lista utenti, lista di carte)
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
#include "timeout-handler.h"
#include "user.h"

#define MAX_PORT ((1 << 16) - 1)

/* ---------- Entità globali (visibili in tutte le unità di compilazione di lavagna) -----------------*/
extern list_t to_do_list; // Contenitore di `card_list_t`
extern list_t doing_list; // Contenitore di `card_list_t`
extern list_t done_list;  // Contenitore di `card_list_t`
extern list_t user_list;  // Contenitore di `user_t`


// Utente identificato dalla porta: viene usata un array
// di puntatori a strutture dati user_t per un accesso rapido
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
 * @brief Costruisce la visualizzazione della lavagna sulla stringa `str`,
 * su una lunghezza massima di `n` caratteri.
 */
void build_lavagna(char *str, size_t n);

/**
 * @brief Costruisce la lista di utenti attualmente connessi sulla stringa `str`,
 * su una lunghezza massima di `n` caratteri. È possibile escludere un utente
 * dalla lista passando il suo descrittore in `excluded_port`
 * 
 */
void build_user_list(char* str, size_t n, user_t *excluded_user);

/**
 * @brief handler per il comando SHOW_LAVAGNA.
 */
void show_lavagna_handler();

/**
 * @brief esegue un comando ricevuto da terminale, già parsato.
 * @param command stringa ricevuta da terminale
 */
void command_dispatch(char *command);

#endif