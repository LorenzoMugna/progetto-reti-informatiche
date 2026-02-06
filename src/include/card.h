#ifndef CARD_H
#define CARD_H

#include <stdint.h>
#include <arpa/inet.h>
#include <time.h>
#include "list.h"

extern uint64_t last_card_id;

typedef struct card_list
{
	list_t list;
	uint64_t ID;		 // ID
	time_t last_changed; // Timestamp ultima modifica
	char *desc;			 // Testo attività
	in_port_t user;		 // Utente (porta) che la\l’ha implementa (“Doing”) \implementata (“Done”)
} card_t;

card_t *new_card(uint64_t ID, char *desc);

void destroy_card(card_t *card);

void clear_card_list(list_t *card_list);

#endif