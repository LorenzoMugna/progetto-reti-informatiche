#ifndef CARD_H
#define CARD_H

#include <stdint.h>
#include <arpa/inet.h>
#include <time.h>
#include "list.h"

typedef struct
{
	uint64_t ID;		 // ID
	char *mess;			 // Testo attività
	in_port_t user;		 // Utente (porta) che la\l’ha implementa (“Doing”) \implementata (“Done”)
	time_t last_changed; // Timestamp ultima modifica

} card_t;

typedef struct cardlist
{
	list_t list_elem;
	card_t card;
} cardlist_t;

#endif