/**
 * @brief Primitive per una gestione sicura della rete 
 * tenendo conto del protocollo applicativo
 */
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
 * @brief riceve un messaggio dal socket `fd` e parsa il comando ricevuto,
 * inserendo il suo indirizzo in `out`.
 * 
 * Se c'è stato un errore nella ricezione, `out` viene impostato a `NULL`
 * e ritorna -1.
 * 
 * Se invece c'è stato un errore solo nel parsing, `out` viene impostato
 * a `NULL` ma ritorna 0.
 * 
 * @note ricordarsi di distruggere il puntatore ricevuto con
 * `destroy_command()`
 * 
 * @returns 0 se la ricezione è andata a buon fine, -1 altrimenti. 
 * Il comando ricevuto viene restituito tramite il parametro `out`.
 * Se c'è stato una errore nel parsing del comando, `out` viene impostato a `NULL`.
 */
int recv_command(int fd, command_t **out);

#endif