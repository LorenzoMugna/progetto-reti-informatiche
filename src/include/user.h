#ifndef USER_H
#define USER_H

#include "card.h"
#include "timeout.h"
#include <arpa/inet.h>

typedef struct user_list
{
	list_t list; // Puntatori per creare la lista
	struct sockaddr_in sockaddr; // Indirizzo dell'utente
	card_t *handled_card;	 // la carta che l'utente sta gestendo (NULL se non sta gestendo carte)
	time_t next_timeout;		 // Il prossimo timeout che dovrà essere gestito per l'utente
	timeout_type_t timeout_type; // Tipo di timeout da gestire
	int socket; // FD del socket associato all'utente
} user_t;


user_t *new_user(struct sockaddr_in *sockaddr, int socket);

void destroy_user(user_t *user);

#endif