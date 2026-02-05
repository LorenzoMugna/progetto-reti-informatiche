#ifndef LAVAGNA_NET_H
#define LAVAGNA_NET_H

#include "net.h"
#include "state-handler.h"
#include "user.h"
#include <sys/time.h>
#include <poll.h>

#define LAVAGNA_PORT 5678
#define CONNECTION_BACKLOG 64
#define _LAVAGNA_TIMEOUT ((struct timeval){5, 0})

#define MAX_USERS 256 // Massimo numero di utenti

/**
 * Usata per definire slot riservati nel `sock_set`
 */
typedef enum reserved_slots{
	RESERVED_STDIN, // Input da terminale
	RESERVED_LISTENER, // Socket per accettare nuove connessioni
	RESERVED_COMMAND_PIPE, // Comandi sulla pipe da altri thread (gestione timeout)
	RESERVED_SOCK_SET_SOCKETS
} reserved_slots_t;

extern struct pollfd sock_set[RESERVED_SOCK_SET_SOCKETS+MAX_USERS]; // set per fare multiplexing I/O sincrono tra
										  // tutte le connessioni tcp aperte
extern uint32_t current_users;

typedef int (*command_handler_t)(user_t *user, command_t* command);

/**
 * Tabella di puntatori a funzione per gestire i comandi ricevuti dai client
 */
extern command_handler_t network_handling_table[N_COMMAND_TOKENS];

/**
 * @brief crea un socket TCP (`SOCK_STREAM`) e lo fa ascoltare
 * su `LAVAGNA_PORT`.
 *
 * @returns il file descriptor del socket appena descritto o
 * -1 in caso di errore.
 */
int init_server();

/**
 * @brief trova un utente nella lista di utenti `user_list`.
 * 
 * @returns puntatore all'elemento della lista se lo trova, `NULL` altrimenti
 */
user_t *find_user_from_fd(int fd);

/**
 * @brief accetta un nuovo utente e attende la ricezione di un HELLO
 * (timeout = `_LAVAGNA_TIMEOUT`).
 * Dopodiché, l'utente viene inserito nella `user_list`.
 *
 * @returns il file descrtiptor associato alla connessione TCP
 * stabilita con il nuovo utente o -1 in caso di errore.
 */
int accept_user(int server_fd);

/**
 * @brief disconnette un utente, rimuovendolo dalla lista di utenti
 */
int disconnect_user(user_t *user);

#endif