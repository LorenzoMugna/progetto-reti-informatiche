/**
 * Funzioni per implementare la stampa sopra il cursore
 * dell'inserimento dei comandi.
 * 
 * Per un riferimento alle sequenze ANSI : https://ansi.tools/lookup
 * '\033' è il carattere con cui iniziano tutte le sequenze ANSI.
 */
#ifndef PRINTING_H
#define PRINTING_H
#include <stdio.h>

/**
 * @brief usata per ottenere la dimensione della console 
 * che sta eseguendo il programma.
 * 
 *
 * @returns il numero di righe nella console attuale o -1 in caso di errore
 */
int get_console_height();

/**
 * @brief utilizza le sequenze di caratteri ANSI per
 * inizializzare la finestra di scorrimento.
 */
void init_printing();

/**
 * @brief ripristina il terminale alla normalità, togliendo la finestra
 * di scorirmento.
 * 
 * @note ricordarsi di chiamare questa funzione all'uscita del programma
 */
void end_printing();

/**
 * @brief permette di stampare sul log che appare sopra il prompt dei comandi
 * 
 * @returns il numero di caratteri scritti
 */
int log_line(const char* format, ...);

/**
 * @brief svuota la riga relativa al prompt dei comandi e reinserisce il `>`
 * 
 * @note questa funzione cancella la riga, va chiamata
 * solo dopo che l'utente ha inviato il comando
 */
void rewrite_prompt();

#endif

