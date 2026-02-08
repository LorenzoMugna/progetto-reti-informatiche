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

	return out > n ? n : out;
}

size_t sprint_cardlist(char *str, size_t n, list_t *b)
{
	size_t printed;
	char *ini_str = str; // (size_t)(str-ini_str) = totale caratteri stampati

	if (n == 0 || !str)
		return 0;

	if (!b || list_empty(b))
	{
		printed = snprintf_wrapper(str, n, "\n");
		str += printed;
		n -= printed;
		return (size_t)(str - ini_str);
	}

	list_t *iter = b->next;

	while (iter != b && n > 0)
	{
		card_t *cardp = (card_t *)iter;

		printed = snprintf_wrapper(str, n, "\t%lu: %s\n", cardp->ID, cardp->desc);
		str += printed;
		n -= printed;

		iter = iter->next;
	}

	return (size_t)(str - ini_str);
}

void build_lavagna(char *str, size_t n)
{
	size_t printed;

	printed = snprintf_wrapper(str, n, "To Do:\n");
	n -= printed;
	str += printed;

	printed = sprint_cardlist(str, n, &to_do_list);
	n -= printed;
	str += printed;

	printed = snprintf_wrapper(str, n, "Doing:\n");
	n -= printed;
	str += printed;

	printed = sprint_cardlist(str, n, &doing_list);
	n -= printed;
	str += printed;

	printed = snprintf_wrapper(str, n, "Done:\n");
	n -= printed;
	str += printed;

	printed = sprint_cardlist(str, n, &done_list);
	n -= printed;
	str += printed;

	printed = snprintf_wrapper(str, n, "-----------------------------\n");
	n -= printed;
	str += printed;
}

void build_user_list(char *str, size_t n, user_t *excluded_user)
{
	size_t printed;
	user_t *user = (user_t *)user_list.next;
	for (;&user->list != &user_list && n > 0;user = (user_t *)user->list.next)
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