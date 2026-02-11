
#include "lavagna-state.h"
#include "lavagna-utils.h"
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

void init_state()
{
	init_list(&to_do_list);
	init_list(&doing_list);
	init_list(&done_list);
	init_list(&user_list);
	last_card_id = 0;
	memset(user_table, 0, sizeof(user_table));
}



void show_lavagna_handler()
{
	char buf[1024];
	build_lavagna(buf, sizeof(buf), 24);
	log_line(buf);
}