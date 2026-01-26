/**
 * @author Lorenzo Mugnaioli
 * @brief definizione funzioni per gestire messaggi scambiati tra gli host
 */
#include "parser.h"

#include <malloc.h>
#include <string.h>

const char *str_command_tokens[] = {
	COMMAND_TOKENS,
	NULL // NULL terminated: riconoscere la fine mentre vengono scorsi tutti i possbili token
};

inline char unescape_char(char c)
{
	switch (c)
	{
	case '\\':
		return '\\';

	case 'n':
		return '\n';

	case '\0':
		return '\0';
	default:
		return '\0';
	}
}

void unescape(char *str)
{
	if (!str)
	{
		return;
	}

	char *read = str;
	char *write = str;
	while (*read)
	{
		*write = *read;
		if (*read == '\\')
		{
			read++;
			*write = unescape_char(*read);
			if (!(*read)) // controlla fine stringa inaspettato
				return;
		}
		read++;
		write++;
	}
}

void trim(char *to_trim)
{
	char *read = to_trim, *write = to_trim;
	if (!read)
		return;
	while (*read == ' ')
	{
		read++;
	}

	while (*read && *read != ' ')
	{
		*write = *read;
		write++;
		read++;
	}

	*write = '\0';
}

command_token_t find_command_id(const char *command_token)
{
	if (!command_token)
		return -1;

	// Ottieni ID comando (le stringhe sono definite nello stesso ordine della enum command_token_t)
	for (int i = 0; str_command_tokens[i]; i++)
	{
		if (!strcmp(str_command_tokens[i], command_token))
		{
			return i;
		}
	}
	return -1;
}

void destroy_command_list(command_t *list)
{
	if (!list)
	{
		return;
	}

	while (!list_empty(&list->param_list))
	{
		command_arg_list_t *elem = (typeof(elem))pop_back(&list->param_list);
		free(elem->buffer);
		free(elem);
	}
}

command_t *parse_command(char *string)
{
	command_t *out = malloc(sizeof(*out));
	out->argc = 0;
	init_list(&out->param_list);

	char *tok_state = NULL;
	char *command = __strtok_r(string, " ", &tok_state);
	trim(command);
	out->command = find_command_id(command);
	if ((int)out->command == -1)
		goto parse_command_error;

	char *tok = __strtok_r(NULL, " ", &tok_state);
	while (tok)
	{
		trim(tok);
		size_t n = strlen(tok);

		// +1 per includere il terminatore null ('\0')
		char *newbuf = malloc(n + 1);
		if (!newbuf)
		{
			goto parse_command_error;
		}

		memcpy(newbuf, tok, n + 1);

		// Crea nuovo elemento nella lista dei comandi
		command_arg_list_t *newarg = malloc(sizeof(*newarg));
		if (!newarg)
		{
			free(newbuf);
			goto parse_command_error;
		}
		push_back(&out->param_list, &newarg->list);

		newarg->buffer = newbuf;
		out->argc++;
		tok = __strtok_r(NULL, " ", &tok_state);
	}

	return out;

	// Gestisci errore: libera risorse allocate
parse_command_error:
	destroy_command_list(out);
	free(out);
	return NULL;
}