#ifndef TIMEOUT_H
#define TIMEOUT_H

#define POLLING_PERIOD 5
#define ACK_CARD_TIMEOUT 5
#define PING_TIMEOUT 90
#define PONG_TIMEOUT 30

typedef enum timeout_type{
	TIMEOUT_NONE,
	TIMEOUT_ACK_CARD, // Aspetta l'ACK_CARD dopo un handle card. Timeout molto corto
	TIMEOUT_PING_UTENTE, // Invia un `PING_UTENTE` allo scadere del tempo
	TIMEOUT_PONG_LAVAGNA // Disconnetti l'utente allo scadere del tempo
} timeout_type_t;

#endif