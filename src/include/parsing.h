/**
 * @author Lorenzo Mugnaioli
 * @brief Definizioni di token usate per il parsing dei messaggi
 * scambiati tra gli host dell'applicazione
 */
#ifndef TOKENS_H
#define TOKENS_H

#include "list.h"

#include <stddef.h>
#include <stdint.h>

// Lista dei possibili comandi
#define COMMAND_TOKENS          \
	TOK(HELLO),                 \
		TOK(QUIT),              \
		TOK(CREATE_CARD),       \
		TOK(MOVE_CARD),         \
		TOK(SHOW_LAVAGNA),      \
		TOK(SEND_USER_LIST),    \
		TOK(PING_USER),         \
		TOK(PONG_LAVAGNA),      \
		TOK(HANDLE_CARD),       \
		TOK(ACK_CARD),          \
		TOK(REQUEST_USER_LIST), \
		TOK(REVIEW_CARD),       \
		TOK(CARD_DONE)

// Lista dei possibili nomi dei comandi
#define PARAM_NAME_TOKENS \
	TOK(SRC)              \
	TOK(DST)              \
	TOK(USER_PORT)        \
	TOK(CARD_ID)

// Definizione della enum di token. Vengono definiti
// elementi con nome corrispondente a ciò che sta dentro TOK
#define TOK(x) x
typedef enum command_token
{
	COMMAND_TOKENS,
	N_COMMAND_TOKENS // Conta il numero di comandi
} command_token_t;
#undef TOK


// Per usi futuri in altri file la lista
// di comandi verrà interpretata come lista di stringhe
#define TOK(x) #x

// Definizione stringhe, da usare con strcmp mentre
// viene parsato il messaggio (definizione in lib/parser.c)
extern const char *str_command_tokens[];


// lista di parametri
typedef struct command_arg_list
{
	list_t list;
	char *buffer;
} command_arg_list_t;

/**
 * @brief Struttura che descrive un comando;
 */
typedef struct command
{
	command_token_t id; // ID del comando
	char *content;
} command_t;

/**
 * @brief ricava id associato al token di un comando. Ritorna (-1) in caso non sia stato trovato
 * @param command_token la stringa relativa al nome del comando, già sottoposta a `strtok()` e `trim()`
 */
command_token_t find_command_id(const char *command_token);

/**
 * @brief libera le risorse associate ad una lista di comandi di un command_t
 * (non libera la struttura stessa del comando)
 * @param command il comando con la lista da liberare
 */
void destroy_command(command_t *command);

/**
 * @brief esegue il parsing di un comando ricevuto da terminale;
 * @returns comando parsato (ID comando + resto del contenuto) o NULL in caso di errore
 */
command_t *parse_command(char *command);
#endif