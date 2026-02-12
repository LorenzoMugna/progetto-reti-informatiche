/**
 * @brief Definizione enumerazione per tipi di timeout
 *
 * I timeout sono usati per gestire le disconnessioni degli utenti inattivi
 * e per gestire i timeout di attesa dell'ACK_CARD dopo un handle card.
 */
#ifndef TIMEOUT_H
#define TIMEOUT_H

typedef enum timeout_type
{
	TIMEOUT_NONE,		 // Nessun timeout attivo
	TIMEOUT_ACK_CARD,	 // Aspetta l'ACK_CARD dopo un handle card.
	TIMEOUT_PING_USER,	 // Invia un `PING_UTENTE` allo scadere del tempo
	TIMEOUT_PONG_LAVAGNA // Disconnetti l'utente allo scadere del tempo
} timeout_type_t;

#endif