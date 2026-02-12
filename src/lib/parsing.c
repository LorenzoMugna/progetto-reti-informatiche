/**
 * @author Lorenzo Mugnaioli
 * @brief definizione funzioni per gestire messaggi scambiati tra gli host
 */
#include "parsing.h"

#include <arpa/inet.h>
#include <malloc.h>
#include <string.h>
#include <stdlib.h>

const char *command_strings[] = {
	COMMAND_TOKENS,
	NULL // NULL terminated: riconoscere la fine mentre vengono scorsi tutti i possbili token
};

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
	for (int i = 0; command_strings[i]; i++)
	{
		if (!strcasecmp(command_strings[i], command_token))
		{
			return i;
		}
	}
	return -1;
}

void destroy_command(command_t *command)
{
	if (command->content)
		free(command->content);

	if (command)
		free(command);
}

command_t *parse_command(char *string)
{
	command_t *out = malloc(sizeof(*out));
	if (!out)
		goto error;
	memset(out, 0, sizeof(*out));

	char *tok_state = NULL;

	char *id_token = strtok_r(string, " \n", &tok_state);
	trim(id_token);
	out->id = find_command_id(id_token);

	if ((int)out->id == -1)
		goto command_created_error;

	size_t n = strlen(tok_state);

	// +1 per includere il terminatore null ('\0')
	char *newbuf = malloc(n + 1);
	if (!newbuf)
		goto command_created_error;

	// Copia il resto del comando 
	memcpy(newbuf, tok_state, n + 1);
	out->content = newbuf;

	return out;

	// Gestisci errore: libera risorse allocate
command_created_error:
	destroy_command(out);
error:
	return NULL;
}

void parse_address(struct sockaddr_in *addr, char *string)
{
	char address[32];
	char port[8];

	memset(address, 0, sizeof(address));
	memset(port, 0, sizeof(port));
	for (uint32_t i = 0; i<sizeof(address) && *string; i++, string++)
	{
		if (*string == ':')
		{
			string++;
			break;
		}
		address[i] = *string;
	}

	for (uint32_t i = 0; i<sizeof(port) && *string; i++, string++)
	{
		port[i] = *string;
	}
	uint16_t port_num = (uint16_t)atoi(port);
	addr->sin_port = htons(port_num);
	addr->sin_family = AF_INET;

	inet_pton(AF_INET, address, &addr->sin_addr);
}