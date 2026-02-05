/**
 * Insieme di funzioni per gestire i timeout.
 * vengono inviati comandi alla pipe
 */
#ifndef TIMEOUT_HANDLER_H
#define TIMEOUT_HANDLER_H

#include "timeout.h"
#include <stdlib.h>
#include <time.h>

#define POLLING_PERIOD 5
#define PING_TIMEOUT 90
#define PONG_TIMEOUT 30


extern int command_pipe[2];


/**
 * @brief Inizializza la pipe per inviare comandi
 * @return fd della parte in lettura
 */
int init_timeout_handler();

/**
 * @brief Inizia il polling
 */
void start_polling();


/**
 * @brief termina il polling
 */
void stop_polling();




#endif