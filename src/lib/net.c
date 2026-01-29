#include "net.h"

#define _NETBUFFER_SIZE (10 * (1 << 10) - 1)

// 10KB - 1 riservato al null terminator:
// ci saranno operazioni su stringhe (vogliamo parsare il
// contenuto del net buffer e il protocollo applicativo è di
// tipo text)
char netbuffer[_NETBUFFER_SIZE + 1];


#define SEND_SIZE sizeof(netbuffer)
#define RECEIVE_SIZE _NETBUFFER_SIZE

#include <stdio.h>

int sendf(int fd, const char *format, ...)
{
	va_list args;
	va_start(args, format);
	vsnprintf(netbuffer, SEND_SIZE, format, args);
	int err = send(fd, netbuffer, strlen(netbuffer) + 1, 0);

	if (err == -1)
	{
		fprintf(stderr, "Errore nell'invio del messaggio.");
	}
	return err;
}

command_t *recv_command(int fd)
{

	recv(fd, netbuffer, RECEIVE_SIZE, 0);
	netbuffer[_NETBUFFER_SIZE] = '\0';
	return parse_command(netbuffer);
}
