#ifndef UTENTE_CLI_H
#define UTENTE_CLI_H

#include "utente-net.h"
#include "parsing.h"

typedef int (*cli_handler_t)(command_t *command);
extern cli_handler_t cli_handlers[N_COMMAND_TOKENS];

/**
 * @brief gestisce un comando ricevuto da stdin secondo la tabella `cli_handlers`
 * @return 0 se ha successo, -1 altrimenti
 */
int cli_event();


#endif