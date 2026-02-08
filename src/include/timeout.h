#ifndef TIMEOUT_H
#define TIMEOUT_H

#define POLLING_PERIOD 1
#define ACK_CARD_TIMEOUT 2
#define PING_TIMEOUT 3
#define PONG_TIMEOUT 3

typedef enum timeout_type{
	TIMEOUT_NONE,
	TIMEOUT_ACK_CARD, // Aspetta l'ACK_CARD dopo un handle card. Timeout molto corto
	TIMEOUT_PING_USER, // Invia un `PING_UTENTE` allo scadere del tempo
	TIMEOUT_PONG_LAVAGNA // Disconnetti l'utente allo scadere del tempo
} timeout_type_t;

#endif