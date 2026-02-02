#include "net.h"
#include "printing.h"

#define _NETBUFFER_SIZE (10 * (1 << 10) - 1)

// 10KB - 1 riservato al null terminator:
// ci saranno operazioni su stringhe (vogliamo parsare il
// contenuto del net buffer e il protocollo applicativo è di
// tipo text)

#define MAX_SEND_SIZE (_NETBUFFER_SIZE + 1)
#define RECEIVE_SIZE _NETBUFFER_SIZE

#include <stdio.h>

typedef struct message
{
	uint32_t content_length;
	char netbuffer[_NETBUFFER_SIZE + 1];
} message_t;

message_t message_buffer;

int sendf(int fd, const char *format, ...)
{
	va_list args;
	va_start(args, format);
	int n = vsnprintf(message_buffer.netbuffer, MAX_SEND_SIZE, format, args);
	va_end(args);

	n = n > MAX_SEND_SIZE ? MAX_SEND_SIZE : n; // Segnala massimo MAX_SEND_SIZE caratteri da leggere

	uint32_t to_send = htonl((uint32_t)n);
	message_buffer.content_length = to_send;

	int err = send(fd, &message_buffer, n + sizeof(message_buffer.content_length), 0);
	if (err == -1)
	{
		fprintf(stderr, "Errore nell'invio del messaggio.");
		return -1;
	}
	return err;
}

command_t *recv_command(int fd)
{
	int ret = recv(fd, &message_buffer, sizeof(message_buffer.content_length), 0);
	if (ret < (int) sizeof(message_buffer.content_length))
		goto error;

	uint32_t to_read = ntohl(message_buffer.content_length);
	message_buffer.netbuffer[to_read] = '\0';
	ret = recv(fd, &message_buffer.netbuffer, (int)to_read, 0);
	if (ret==-1 || (uint32_t) ret < to_read || ret == 0)
		goto error;

	message_buffer.netbuffer[_NETBUFFER_SIZE] = '\0';
	return parse_command(message_buffer.netbuffer);

error:
	log_line("Errore nella ricezione del messaggio");
	return NULL;
}
