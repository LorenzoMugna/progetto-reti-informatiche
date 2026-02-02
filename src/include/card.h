#ifndef CARD_H
#define CARD_H

#include <stdint.h>
#include <arpa/inet.h>
#include <time.h>
#include "list.h"

extern uint64_t last_card_id;

typedef struct
{
	uint64_t ID;		 // ID
	char *mess;			 // Testo attività
	in_port_t user;		 // Utente (porta) che la\l’ha implementa (“Doing”) \implementata (“Done”)
	time_t last_changed; // Timestamp ultima modifica

} card_t;

typedef struct card_list
{
	list_t list_elem;
	card_t card;
} card_list_t;

#endif