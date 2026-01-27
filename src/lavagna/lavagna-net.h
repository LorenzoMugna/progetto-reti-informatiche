#ifndef LAVAGNA_NET_H
#define LAVAGNA_NET_H

#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/time.h>
#include <unistd.h>

#define LAVAGNA_PORT 5678
#define CONNECTION_BACKLOG 64
#define _LAVAGNA_TIMEOUT ((struct timeval){5,0})

//10KB - 1 riservato al null terminator:
//ci saranno operazioni su stringhe (vogliamo parsare il
//contenuto del net buffer e il protocollo applicativo è di
//tipo text)
#define NETBUFFER_SIZE (10*(1<<10)-1) 
extern char netbuffer[NETBUFFER_SIZE+1];

/**
 * @brief crea un socket TCP (`SOCK_STREAM`) e lo fa ascoltare
 * su `LAVAGNA_PORT`.
 * 
 * @returns il file descriptor del socket appena descritto o 
 * -1 in caso di errore.
 */
int init_server_socket();

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