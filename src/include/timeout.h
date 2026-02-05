#ifndef TIMEOUT_H
#define TIMEOUT_H

typedef enum timeout_type{
	TIMEOUT_NONE,
	TIMEOUT_PING_UTENTE, // Invia un `PING_UTENTE` allo scadere del tempo
	TIMEOUT_PONG_LAVAGNA // Disconnetti l'utente allo scadere del tempo
} timeout_type_t;

#endif