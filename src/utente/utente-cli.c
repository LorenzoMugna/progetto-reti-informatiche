#include "parsing.h"
#include "printing.h"
#include "utente-cli.h"
#include "utente-net.h"

#include <signal.h>

int cli_event()
{
	char buf[256];
	if (!fgets(buf, sizeof(buf), stdin))
		goto error;
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
	return 0;
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

	int err = sendf(my_socket, "%s %s", str_command_tokens[CREATE_CARD], command->content);
	if (err == -1)
		goto error;

	return 0;
error:
	return -1;
}

int cli_handle_SHOW_LAVAGNA(command_t *command)
{
	(void)command;

	int err = sendf(my_socket, "%s", str_command_tokens[SHOW_LAVAGNA]);
	if (err == -1)
		goto error;

	return 0;

error:
	return -1;
}

int cli_handle_REVIEW_CARD(command_t *command)
{
	(void)command;

	if (current_user_state != STATE_HANDLING)
	{
		log_line("Non si sta gestendo nessuna carta, impossibile inviare review\n");
		goto error;
	}
	current_user_state = STATE_GETTING_USER_LIST;
	// Richiedi lista utenti  
	int err = sendf(my_socket, "%s", str_command_tokens[REQUEST_USER_LIST]);
	if (err == -1)
		goto error;

	// Il resto viene gestito alla ricezione di un SEND_USER_LIST
	// in utente-cli.c: handle_SEND_USER_LIST 
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

	int err = sendf(my_socket, "%s", str_command_tokens[CARD_DONE]);
	if (err == -1)
		goto error;

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
	[CARD_DONE] = cli_handle_CARD_DONE,};