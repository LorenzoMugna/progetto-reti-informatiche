#include "user.h"
#include <unistd.h>
#include <string.h>

user_t *new_user(struct sockaddr_in *sockaddr, int socket)
{
	user_t *user = malloc(sizeof(*user));
	if(!user)
		goto error;
	
	init_list(&user->list);
	memcpy(&user->sockaddr, sockaddr, sizeof(*sockaddr));
	user->handled_card = NULL;
	user->next_timeout = 0;
	user->timeout_type = TIMEOUT_NONE;
	user->socket = socket;

	return user;

error:
	return NULL;
}

void destroy_user(user_t *user)
{
	if (!user)
		return;
	
	pop_elem(&user->list);
	close(user->socket);
	free(user);
}

void clear_user_list(list_t *user_list)
{
	if (!user_list)
		return;

	while (!list_empty(user_list))
	{
		user_t *user = (user_t *)user_list->next;
		destroy_user(user);
	}
}