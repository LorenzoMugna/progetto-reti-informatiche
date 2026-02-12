#include "lavagna-cli.h"
#include "lavagna-state.h"
#include "lavagna-utils.h"
#include "parsing.h"
#include "printing.h"
#include "timeout.h"

#include <signal.h>
#include <string.h>

const char helpstring[] =
	"Comandi disponibili:\n"
	"QUIT                            termina l'esecuzione della lavagna e degli utenti\n"
	"CREATE_CARD <testo>             crea una nuova carta con il testo specificato e inseriscila\n"
	"                                in To Do\n"
	"SHOW_LAVAGNA                    mostra la lavagna\n"
	"PING_USER <porta>               invia un PING_USER manualmente ad un utente in qualsiasi momento,\n"
	"                                anche quando non sta gestendo una carta\n"
	"HELP                            mostra questo messaggio di aiuto\n"
	"\n"
	"Tutti i comandi sono case-insensitive.\n\n"

	;
int cli_event()
{
	char buf[256];
	if (!fgets(buf, sizeof(buf), stdin))
		goto error;
	if (strcasecmp(buf, "help\n") == 0)
	{
		log_line(helpstring);
		return 0;
	}

	command_t *command = parse_command(buf);
	if (!command)
		goto error;

	cli_handler_t handler = cli_handlers[command->id];
	if (!handler)
		goto command_created_error;

	int err = handler(command);

	destroy_command(command);
	return err;

command_created_error:
	destroy_command(command);
error:
	return -1;
}

int cli_ignore_command(command_t *command)
{
	(void)command;
	return -1; // Ritorna -1 per segnalare che il comando non è previsto per la CLI
}

int cli_handle_QUIT(command_t *command)
{
	(void)command;

	// Termina semplicemente l'esecuzione:
	// la liberazione delle risorse avviene alla fine del main
	kill(getpid(), SIGINT);
	return 0;
}

int cli_handle_CREATE_CARD(command_t *command)
{
	if (!command || !command->content)
		goto error;

	card_t *card = new_card(last_card_id+1, command->content);
	if (!card)
		goto error;

	last_card_id++;
	push_back(&to_do_list, &card->list);
	distribute_cards();
	show_lavagna();
	return 0;

error:
	return -1;
}

int cli_handle_SHOW_LAVAGNA(command_t *command)
{
	(void)command;

	show_lavagna();
	return 0;
}

int cli_handle_PING_USER(command_t *command)
{
	(void)command;

	char *token_state = NULL;
	char *port_token = strtok_r(command->content, " ", &token_state);
	if (!port_token)
	{
		log_line("Formato non valido per PING_USER\n");
		goto error;
	}

	uint16_t port = (uint16_t)atoi(port_token);
	user_t *user = user_table[port];
	if (!user)
	{
		log_line("Porta %hu non associata ad alcun utente, impossibile inviare PING_USER\n", port);
		goto error;
	}

	int err = sendf(user->socket, "%s", command_strings[PING_USER]);
	if (err == -1)
		goto error;

	user->next_timeout = time(NULL) + PONG_TIMEOUT;
	user->timeout_type = TIMEOUT_PONG_LAVAGNA;

	log_line("[PING_USER] -> %hu\n", port);
	return 0;

error:
	return -1;
}


cli_handler_t cli_handlers[N_COMMAND_TOKENS] = {
	[HELLO] = cli_ignore_command,
	[QUIT] = cli_handle_QUIT,
	[CREATE_CARD] = cli_handle_CREATE_CARD,
	[MOVE_CARD] = cli_ignore_command,
	[SHOW_LAVAGNA] = cli_handle_SHOW_LAVAGNA,
	[SEND_USER_LIST] = cli_ignore_command,
	[PING_USER] = cli_handle_PING_USER,
	[PONG_LAVAGNA] = cli_ignore_command,
	[HANDLE_CARD] = cli_ignore_command,
	[ACK_CARD] = cli_ignore_command,
	[REQUEST_USER_LIST] = cli_ignore_command,
	[REVIEW_CARD] = cli_ignore_command,
	[CARD_DONE] = cli_ignore_command};
