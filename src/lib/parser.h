#ifndef PARSER_H
#define PARSER_H
#include "parsing.h"


/**
 * @brief funzione per effettuare l'escape di un singolo carattere
 * @param c il carattere che segue un backslash (\) in una stringa
 * @returns il corrispondente carattere di escape
 */
char unescape_char(char c);

/**
 * @brief Sostituisce in-place le sequenze di caratteri di un escape
 * con i caratteri che esse rappresentano
 */
void unescape(char* s);

/**
 * @brief sostituisce alcuni caratteri con la sequenza di escape correlata
 */
char* escape(char* dest, char *src, size_t n);



#endif