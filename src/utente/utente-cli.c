#include "parsing.h"
#include "printing.h"
#include "utente-cli.h"
#include "utente-net.h"

#include <signal.h>

const char helpstring[] =
	"Comandi disponibili:\n"
	"QUIT                            termina l'esecuzione dell' utente attuale\n"
	"SHOW_LAVAGNA                    richiedi la visualizzazione alla lavagna e mostrala\n"
	"CREATE_CARD <testo>             crea una nuova carta con il testo specificato e inseriscila\n"
	"                                in To Do\n"
	"REVIEW_CARD                     inizia manualmente la fase di revisione di una carta nel caso\n"
	"                                il primo tentativo fallisca. Verrà inviato automaticamente il CARD_DONE\n"
	"                                alla lavagna\n"
	"                                quando tutti gli utenti avranno accettato la carta\n"
	"CARD_DONE                       invia manualmente CARD_DONE nel caso ci sia stato un errore la prima volta\n"
	"HELP                            mostra questo messaggio di aiuto\n"

	"\n"
	"Tutti i comandi sono case-insensitive.\n\n";

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

	if (command->id >= N_COMMAND_TOKENS)
		goto command_created_error;

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
	if (current_user_state == STATE_REVIEWING)
	{
		log_line("Impossibile uscire si stanno aspettando le review\n");
		return -1;
	}

	log_line("Fine sessione\n");
	pid_t pid = getpid();
	kill(pid, SIGINT);
	return 0;
}

int cli_handle_SHOW_LAVAGNA(command_t *command);

int cli_handle_CREATE_CARD(command_t *command)
{
	if (!command || !command->content)
		goto error;

	int err = sendf(my_socket, "%s %s", command_strings[CREATE_CARD], command->content);
	if (err == -1)
		goto error;

	return 0;
error:
	return -1;
}

int cli_handle_SHOW_LAVAGNA(command_t *command)
{
	(void)command;

	int err = sendf(my_socket, "%s", command_strings[SHOW_LAVAGNA]);
	if (err == -1)
		goto error;

	return 0;

error:
	return -1;
}

int cli_handle_REVIEW_CARD(command_t *command)
{
	(void)command;

	if (current_user_state == STATE_DONE)
	{
		log_line("Si è già pronti per mandare CARD_DONE, non è necessario richiedere nuovamente la review.\n");
		goto error;
	}

	if (current_user_state != STATE_HANDLING)
	{
		log_line("Non si sta gestendo nessuna carta, impossibile inviare review\n");
		goto error;
	}

	// Richiedi lista utenti
	int err = sendf(my_socket, "%s", command_strings[REQUEST_USER_LIST]);
	if (err == -1)
		goto error;

	current_user_state = STATE_GETTING_USER_LIST;

	// Il resto viene gestito alla ricezione di un SEND_USER_LIST
	// in utente-net.c: handle_SEND_USER_LIST

	// La logica per la ricezione di approvazioni è presente in utente-net.c:accept_request
	return 0;

error:
	return -1;
}

int cli_handle_CARD_DONE(command_t *command)
{
	(void)command;

	if (current_user_state != STATE_DONE)
	{
		log_line("Non si è ancora pronti per mandare CARD_DONE\n");
		goto error;
	}

	int err = sendf(my_socket, "%s", command_strings[CARD_DONE]);
	if (err == -1)
		goto error;

	destroy_card(handled_card);
	handled_card = NULL;
	current_user_state = STATE_IDLE;
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
	[PING_USER] = cli_ignore_command,
	[PONG_LAVAGNA] = cli_ignore_command,
	[HANDLE_CARD] = cli_ignore_command,
	[ACK_CARD] = cli_ignore_command,
	[REQUEST_USER_LIST] = cli_ignore_command,
	[REVIEW_CARD] = cli_handle_REVIEW_CARD,
	[CARD_DONE] = cli_handle_CARD_DONE,
};