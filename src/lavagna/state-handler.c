
#include "state-handler.h"
#include "lavagna-net.h"
#include "printing.h"

#include <stdarg.h>

/* -------------- Definizione entità globali ------------------*/
/* --(visibili in tutte le unità di compilazione di lavagna) -- */

list_t to_do_list;
list_t doing_list;
list_t done_list;
list_t user_list;
uint64_t last_card_id;
user_t *user_table[MAX_PORT];

/**
 * @brief `snprintf()` ritorna il numero di caratteri
 * che avrebbe scritto se avesse avuto spazio a sufficienza.
 * Questa funzione consente di ritornare il numeri di caratteri
 * effettivamente scritti
 */
static inline size_t snprintf_wrapper(char *restrict str, size_t n, char *restrict format, ...)
{
	va_list arglist;
	va_start(arglist, format);
	size_t out = vsnprintf(str, n, format, arglist);
	va_end(arglist);

	return out>n? n:out;
}

void init_state(){
	init_list(&to_do_list);
	init_list(&doing_list);
	init_list(&done_list);
	init_list(&user_list);
	last_card_id = 0;
	memset(user_table, 0, sizeof(user_table));
}

size_t sprint_cardlist(char *str, size_t n, list_t *b)
{
	size_t printed;
	char *ini_str = str; // (int)(str-ini_str) = totale caratteri stampati

	if (n==0 || !str)
		return 0;

	if (!b || list_empty(b))
	{
		printed = (size_t)snprintf(str, n, "\n");
		printed = (size_t) printed > n? n : printed;
		str += printed;
		n -= printed;
		return (int)(str-ini_str);
	}

	list_t *iter = b->next;

	while (b != iter && n>0)
	{
		card_t *cardp = (card_t *)iter;

		printed = (size_t)snprintf(str, n,"\t%lu: %s\n", cardp->ID, cardp->desc);
		printed = (size_t) printed > n ? n : printed;
		str += printed;
		n -= printed;

		iter = iter->next;
	}

	return (size_t)(str-ini_str);
}

void build_lavagna(char* str, size_t n)
{
	size_t printed;

	printed = snprintf_wrapper(str,n,"To Do:\n");
	n-=printed;
	str+=printed;

	printed = sprint_cardlist(str,n,&to_do_list);
	n-=printed;
	str+=printed;

	printed = snprintf_wrapper(str, n, "Doing:\n");
	n-=printed;
	str+=printed;

	printed = sprint_cardlist(str,n,&doing_list);
	n-=printed;
	str+=printed;

	printed = snprintf_wrapper(str, n, "Done:\n");
	n-=printed;
	str+=printed;

	printed = sprint_cardlist(str,n,&done_list);
	n-=printed;
	str+=printed;

	printed = snprintf_wrapper(str, n, "-----------------------------\n");
	n-=printed;
	str+=printed;
}

void show_lavagna_handler()
{
	char buf[1024];
	build_lavagna(buf, sizeof(buf));
	log_line(buf);
}

void build_user_list(char* str, size_t n)
{
	user_t *user = (user_t*)user_list.next;
	size_t printed;
	while(&user->list != &user_list && n>0)
	{
		char inet_buffer[20];
		inet_ntop(AF_INET, &user->sockaddr.sin_addr, inet_buffer, sizeof(inet_buffer));
		uint16_t port = ntohs(user->sockaddr.sin_port);
		printed = snprintf_wrapper(str, n, "%s:%u ", inet_buffer, port);
		str+=printed;
		n-=printed;

		user = (user_t*)user->list.next;
	}
}