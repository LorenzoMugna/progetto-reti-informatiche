#ifndef UTENTE_NET_H
#define UTENTE_NET_H

#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <time.h>
#include "net.h"

#define MAX_BACKLOG 8

#define _UTENTE_TIMEOUT ((struct timeval){1,0})

#define ACCEPT_TIMER ((struct timespec){1, 0})

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
int init_listener_socket(uint16_t port);

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