#ifndef LAVAGNA_NET_H
#define LAVAGNA_NET_H

#include <arpa/inet.h>
#include <sys/socket.h>

#define LAVAGNA_PORT 5678
#define CONNECTION_BACKLOG 64


/**
 * @brief crea un socket TCP (`SOCK_STREAM`) e lo fa ascoltare
 * su `LAVAGNA_PORT`.
 * 
 * @returns il file descriptor del socket appena descritto o 
 * -1 in caso di errore.
 */
int init_server_socket();

/**
 * @brief accetta un nuovo utente e attende la ricezione di un HELLO.
 * Dopodiché, l'utente viene inserito nella `user_list`.
 * 
 * @returns il file descrtiptor associato alla connessione TCP
 * stabilita con il nuovo utente o -1 in caso di errore.
 */
int accept_user(int server_fd);

#endif