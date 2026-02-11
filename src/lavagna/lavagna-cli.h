#ifndef LAVAGNA_CLI_H
#define LAVAGNA_CLI_H

#include "parsing.h"

typedef int (*cli_handler_t)(command_t* command);

extern cli_handler_t cli_handlers[N_COMMAND_TOKENS];

/**
 * @brief da chiamare quando ci sono dati da leggere da stdin,
 * gestisce l'input dell'utente
 * 
 * @returns 0 se ha successo, -1 altrimenti
 */
int cli_event();


#endif