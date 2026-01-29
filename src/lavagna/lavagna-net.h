#ifndef LAVAGNA_NET_H
#define LAVAGNA_NET_H

#include "net.h"
#include <sys/time.h>
#include <poll.h>

#define LAVAGNA_PORT 5678
#define CONNECTION_BACKLOG 64
#define _LAVAGNA_TIMEOUT ((struct timeval){5, 0})

#define MAX_USERS 256 // Massimo numero di utenti


extern struct pollfd sock_set[MAX_USERS]; // set per fare multiplexing I/O sincrono tra
										  // tutte le connessioni tcp aperte
extern uint32_t current_users;

/**
 * @brief crea un socket TCP (`SOCK_STREAM`) e lo fa ascoltare
 * su `LAVAGNA_PORT`.
 *
 * @returns il file descriptor del socket appena descritto o
 * -1 in caso di errore.
 */
int init_server();

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