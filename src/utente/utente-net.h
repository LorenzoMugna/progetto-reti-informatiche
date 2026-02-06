#ifndef UTENTE_NET_H
#define UTENTE_NET_H

#include <arpa/inet.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <time.h>

#include "net.h"
#include "list.h"

#define MAX_BACKLOG 8

#define _UTENTE_TIMEOUT ((struct timeval){1, 0})

#define ACCEPT_TIMER ((struct timespec){1, 0})

typedef int (*network_handler_t)(command_t *command);

typedef enum user_state
{
	STATE_IDLE,				 // L'utente non sta gestendo carte
	STATE_HANDLING,			 // L'utente ha una carta in possesso
	STATE_GETTING_USER_LIST, // L'utente sta richiedendo la lista degli utenti
	STATE_REVIEWING,		 // L'utente sta aspettando le review
	STATE_DONE				 // L'utente ha finito e può mandare CARD_DONE
} user_state_t;

extern network_handler_t network_handlers[N_COMMAND_TOKENS];

typedef struct useraddr{
	list_t list;
	struct sockaddr_in user_address;
} useraddr_t;

useraddr_t *new_useraddr();

void destroy_useraddr(useraddr_t *useraddr);

/**
 * @brief Inizializza un socket per il client
 * e stabilisce una connessione con la lavagna (Inviando un HELLO e
 * attendendo l'HELLO di risposta)
 * @param port la porta da usare per la connessione (default 5679)
 * @return fd del socket se ha successo, -1 altrimenti
 */
int init_socket(uint16_t port);

/**
 * @brief Inizializza un socket che permette di ricevere richieste
 * di connessione da parte di altri utent
 *
 * @return fd del socket se ha successo, -1 altrimenti
 */
int init_listener_socket();

/**
 * @brief accetta le richieste di connessione dagli altri utenti.
 * Si hanno poi due strade in base al comando ricevuto:
 *
 *
 * @return 0 se ha successo, -1 altrimenti
 */
int accept_request(int fd);

/**
 * @brief funzione eseguita dal thread responsabile di inviare le
 * risposte alle richieste di review
 */
void *review_response(void *arg);
#endif