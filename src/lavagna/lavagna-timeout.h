/**
 * Insieme di funzioni per gestire i timeout.
 * vengono inviati comandi alla pipe
 */
#ifndef TIMEOUT_HANDLER_H
#define TIMEOUT_HANDLER_H

#include "timeout.h"
#include <stdlib.h>
#include <time.h>



extern int timeout_pipe[2];


/**
 * @brief Inizializza la pipe per inviare comandi
 * @return fd della parte in lettura
 */
int init_timeout_handler();

/**
 * @brief libera le risorse allocate per il timeout handler
 * (chiude i file descriptor della pipe e rimuove il fd dal sock_set)
 */
void destroy_timeout_handler();

/**
 * @brief Inizia il polling
 */
void start_polling();


/**
 * @brief termina il polling
 */
void stop_polling();


/**
 * @brief Callback per gestire i timeout
 */
void polling_handler();



#endif