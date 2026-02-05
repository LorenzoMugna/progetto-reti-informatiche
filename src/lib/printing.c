#include "printing.h"
#include <sys/ioctl.h>
#include <unistd.h>
#include <stdarg.h>

int console_height;

int get_console_height()
{
	struct winsize w;
	if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &w) == -1)
	{
		fprintf(stderr, "Errore durante l'ottenimento delle dimensioni"
						" della finestra. Default messo a 15");
		console_height = 15;
		return -1;
	}

	console_height = w.ws_row;
	return console_height;
}

void init_printing()
{
	// Pulisci schermo, inizializza la finestra di scorrimento
	// e stampa il prompt per il comando
	get_console_height();
	printf("\033[2J\033[1;%dr", console_height - 1);
	rewrite_prompt();
	fflush(stdout);
}

void end_printing()
{
	// togli finestra di scorrimento, posiziona il cursore in fondo e 
	// vai ad una riga nuova
	printf("\033[r\033[%d;1H\n", console_height);
	fflush(stdout);
}

void rewrite_prompt()
{
	//Posiziona il cursore all'inizio della riga del prompt,
	//Cancellala e riscrivi il prompt
	printf("\033[%d;1H\033[2K> ", console_height);
	fflush(stdout);
}

int log_line(const char *format, ...)
{
	va_list args;
	va_start(args, format);

	// Posiziona il cursore nella parte del log
	printf("\033[s\033[%d;1H", console_height-1); 
	int ret = vprintf(format, args);
	// Ripristina posizione del cursore
	printf("\033[u\033[%dd", console_height);
	va_end(args);
	fflush(stdout);
	return ret;
}
