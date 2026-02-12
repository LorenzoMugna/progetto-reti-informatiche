/**
 * @brief Funzioni di utilità varie per la lavagna
 */
#ifndef LAVAGNA_UTILS_H
#define LAVAGNA_UTILS_H

#include "lavagna-state.h"
#include "lavagna-net.h"
#include "printing.h"

#include <stdint.h>

#define LAVAGNA_COLUMN_WIDTH 30
/**
 * @brief Costruisce la visualizzazione della lavagna sull buffer `str`
 * lungo `n` caratteri.
 */
void build_lavagna(char *str, size_t n, size_t column_width);

/**
 * @brief Costruisce la lista di utenti attualmente connessi sulla stringa `str`,
 * su una lunghezza massima di `n` caratteri. È possibile escludere un utente
 * dalla lista passando il suo descrittore in `excluded_list`
 * 
 */
void build_user_list(char* str, size_t n, user_t *excluded_user);

/**
 * @brief handler per il comando SHOW_LAVAGNA.
 */
void show_lavagna();

/**
 * @brief Prova a distribuire le carte non assegnate agli utenti che non stanno gestendo carte,
 * rispettando l'ordine di porta che è dato dall'ordinamento di `user_list`.
 */
void distribute_cards();

#endif