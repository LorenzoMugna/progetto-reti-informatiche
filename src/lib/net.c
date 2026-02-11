#include "net.h"
#include "printing.h"


#include <stdio.h>

typedef struct message
{
	uint16_t content_length;
	char netbuffer[MAX_SEND_SIZE+1]; // +1 perché il terminatore di stringa non è incluso
} message_t;

static message_t message_buffer;

int sendf(int fd, const char *format, ...)
{
	va_list args;
	va_start(args, format);
	int n = vsnprintf(message_buffer.netbuffer, sizeof(message_buffer.netbuffer), format, args);
	va_end(args);

	n = n > MAX_SEND_SIZE ? MAX_SEND_SIZE : n; // Segnala massimo MAX_SEND_SIZE caratteri da leggere

	uint16_t to_send = htons((uint16_t)n);
	message_buffer.content_length = to_send;

	int err = send(fd, &message_buffer, n + sizeof(message_buffer.content_length), 0);
	if (err == -1)
	{
		log_line("Errore nell'invio del messaggio.\n");
		return -1;
	}
	return err;
}

int recv_command(int fd, command_t **out)
{
	int ret = recv(fd, &message_buffer, sizeof(message_buffer.content_length), 0);
	if (ret < (int) sizeof(message_buffer.content_length))
		goto error;

	uint16_t to_read = ntohs(message_buffer.content_length);
	message_buffer.netbuffer[to_read] = '\0';
	ret = recv(fd, &message_buffer.netbuffer, (int)to_read, 0);
	if (ret==-1 || (uint32_t) ret < to_read || ret == 0)
		goto error;

	message_buffer.netbuffer[_NETBUFFER_SIZE] = '\0';
	*out = parse_command(message_buffer.netbuffer);
	return 0;

error:
	*out = NULL;
	return -1;
}
