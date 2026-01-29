#ifndef UTENTE_NET_H
#define UTENTE_NET_H

#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include "net.h"

/**
 * @brief Inizializza un socket per il client 
 * e stabilisce una connessione con la lavagna (Inviando un HELLO e 
 * attendendo l'HELLO di risposta)
 * @param port la porta da usare per la connessione (default 5679)
 * @return fd del socket se ha successo, -1 altrimenti
 */
int init_socket(uint16_t port);

#endif