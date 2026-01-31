#ifndef LAVAGNA_NET_H
#define LAVAGNA_NET_H

#include "net.h"
#include "state-handler.h"
#include <sys/time.h>
#include <poll.h>

#define LAVAGNA_PORT 5678
#define CONNECTION_BACKLOG 64
#define _LAVAGNA_TIMEOUT ((struct timeval){5, 0})

#define MAX_USERS 256 // Massimo numero di utenti

#define RESERVED_SOCK_SET_SOCKETS 2 // 2 socket riservati (stdin e listening server socket)
#define RESERVED_STDIN 0
#define RESERVED_LISTENER 1

extern struct pollfd sock_set[MAX_USERS]; // set per fare multiplexing I/O sincrono tra
										  // tutte le connessioni tcp aperte
extern uint32_t current_users;

typedef int (*command_handler_t)(user_list_t *user,command_t* command);

extern command_handler_t command_handling_table[N_COMMAND_TOKENS];

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
user_list_t *find_user_from_fd(int fd);

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
void disconnect_user(uint16_t user_port);

#endif