#include "lavagna-utils.h"

/**
 * @brief `snprintf()` ritorna il numero di caratteri
 * che _avrebbe_ scritto se avesse avuto spazio a sufficienza.
 * Questa funzione consente di ritornare il numero di caratteri
 * effettivamente scritti
 */
static inline size_t snprintf_wrapper(char *restrict str, size_t n, char *restrict format, ...)
{
	va_list arglist;
	va_start(arglist, format);
	size_t out = vsnprintf(str, n, format, arglist);
	va_end(arglist);

	return out >= n ? n - 1 : out;
}

/**
 * @brief Funzione di utilità per stampare una carta in una colonna.
 * Viene gestito il caso di una descrizione che non rientra in una sola riga.
 * 
 * @note `list_iter` e `desc_iter` devono essere modificati, per questo sono puntatori doppi.
 */
static void iter_printing(char *str, size_t column_width, list_t **list_iter, char **desc_iter)
{
	size_t printed;
	if (*desc_iter)
	{
		// Continua la stampa
		printed = snprintf_wrapper(str, column_width, "%s", *desc_iter);
		*desc_iter += printed;
		if (!**desc_iter)
		{
			// Fine carta, passa alla prossima
			*list_iter = (*list_iter)->next;
			*desc_iter = NULL;
		}
	}
	else
	{
		// Inizia nuova carta
		card_t *cardp = (card_t *)*list_iter;
		*desc_iter = cardp->desc;
		printed = snprintf_wrapper(str, column_width, "%lu [%hu]: ", cardp->ID, cardp->user);
		printed = snprintf_wrapper(str + printed, column_width - printed, "%s", *desc_iter);
		*desc_iter += printed;
		if (!**desc_iter)
		{
			// Fine carta, passa alla prossima
			*list_iter = (*list_iter)->next;
			*desc_iter = NULL;
		}
	}
}

/**
 * @brief stampa una riga di separazione includendo eventualmente le intersezioni (`+`)
 * @returns il numero di caratteri stampati
 */
static size_t print_sep_line(char *str, size_t n, size_t column_width, bool intersections)
{
	size_t printed;
	char *str_ini = str;
	for (size_t i = 0; i < 3u*column_width; i++)
	{
		if(i%(column_width+1) == column_width && intersections)
		{
			printed = snprintf_wrapper(str, n, "+");
		}
		else
		{
			printed = snprintf_wrapper(str, n, "-");
		}
		str += printed;
		n -= printed;
	}
	printed = snprintf_wrapper(str, n, "\n");
	str += printed;
	n -= printed;
	return (size_t)(str - str_ini);
}

void build_lavagna(char *str, size_t n, size_t column_width)
{
	size_t printed;

	printed = print_sep_line(str, n, column_width, true);
	str += printed;
	n -= printed;

	printed = snprintf_wrapper(str, n, "%-*s|%-*s|%-*s\n",
							   column_width, "To Do",
							   column_width, "Doing",
							   column_width, "Done");
	n -= printed;
	str += printed;

	printed = print_sep_line(str, n, column_width, true);
	str += printed;
	n -= printed;

	list_t *to_do_iter = to_do_list.next;
	list_t *doing_iter = doing_list.next;
	list_t *done_iter = done_list.next;
	char *to_do_iter_pointer = NULL;
	char *doing_iter_pointer = NULL;
	char *done_iter_pointer = NULL;
	while ((to_do_iter != &to_do_list || doing_iter != &doing_list || done_iter != &done_list) && n > 0)
	{
	char to_do_str[25] = {0},
		 doing_str[25] = {0},
		 done_str[25] = {0};

		if (to_do_iter != &to_do_list)
		{
			iter_printing(to_do_str, column_width, &to_do_iter, &to_do_iter_pointer);
		}

		if (doing_iter != &doing_list)
		{
			iter_printing(doing_str, column_width, &doing_iter, &doing_iter_pointer);
		}

		if (done_iter != &done_list)
		{
			iter_printing(done_str, column_width, &done_iter, &done_iter_pointer);
		}
		printed = snprintf_wrapper(str, n, "%-*s|%-*s|%-*s\n",
								   column_width, to_do_str,
								   column_width, doing_str,
								   column_width, done_str);
		str += printed;
		n -= printed;
	}
	print_sep_line(str, n, column_width, true);

}

void build_user_list(char *str, size_t n, user_t *excluded_user)
{
	size_t printed;
	user_t *user = (user_t *)user_list.next;
	for (; &user->list != &user_list && n > 0; user = (user_t *)user->list.next)
	{

		if (user == excluded_user)
			continue;
		uint16_t port = ntohs(user->sockaddr.sin_port);
		char inet_buffer[20];
		inet_ntop(AF_INET, &user->sockaddr.sin_addr, inet_buffer, sizeof(inet_buffer));
		printed = snprintf_wrapper(str, n, "%s:%u ", inet_buffer, port);
		log_line("%s\n", str);
		str += printed;
		n -= printed;
	}
}

int send_card_to_handle(user_t *user, card_t *card)
{
	if (!user || !card)
		goto error;

	// Invia la carta all'utente, assegnala preventivamente
	// e imposta un timeout per l'ACK_CARD
	char buf[1024];
	build_user_list(buf, sizeof(buf), user);
	int err = sendf(user->socket, "%s %u %s %s",
					command_strings[HANDLE_CARD],
					current_users,
					buf,
					card->desc);
	if (err == -1)
		goto error;

	uint16_t user_port = ntohs(user->sockaddr.sin_port);
	card->user = user_port;
	user->timeout_type = TIMEOUT_ACK_CARD;
	user->next_timeout = time(NULL) + ACK_CARD_TIMEOUT;
	user->handled_card = card;

	// sposta la carta in doing solo dopo la ricezione dell'ACK_CARD.
	// in caso di timeout queste modifiche vengono annullate
	log_line("[HANDLE_CARD] %llu: %s -> %hu\n", card->ID, card->desc, user_port);
	return 0;
error:
	return -1;
}


void distribute_cards()
{
	// if (current_users < 2)
	// 	return;

	// Per ogni utente, se non sta gestendo carte, dagli una carta da fare
	list_t *iter = user_list.next;
	list_t *card_iter = to_do_list.next;
	while (iter != &user_list)
	{
		user_t *user = (user_t *)iter;
		if (user->handled_card)
		{
			iter = iter->next;
			continue;
		}

		while (card_iter != &to_do_list)
		{
			card_t *card = (card_t *)card_iter;
			if (card->user == 0) // Carta non assegnata
			{
				send_card_to_handle(user, card);
				card_iter = card_iter->next;
				break;
			}
			card_iter = card_iter->next;
		}
		iter = iter->next;
	}
}