#ifndef NET_H
#define NET_H

#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdarg.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#include "parsing.h"



/**
 * @brief invia un messaggio sul socket `fd`, seguendo il formato
 * specificato e prependendo la lunghezza del contenuto.
 * 
 * @return il numero di byte inviati se ha successo, -1 altrimenti
 */
int sendf(int fd, const char* format, ...);


/**
 * @brief riceve un messaggio dal socket `fd`
 * @note ricordarsi di distruggere il puntatore ricevuto con
 * `destroy_command_list()` e `free()`
 * @returns puntatore al comando parsato o NULL in caso di errore
 */
command_t *recv_command(int fd);

#endif