#include <arpa/inet.h>
#include <netinet/in.h>
#include <pthread.h>
#include <signal.h>
#include <sys/select.h>
#include <sys/socket.h>

#include "utente-net.h"
#include "utente-cli.h"
#include "card.h"
#include "parsing.h"
#include "printing.h"

int my_socket;
struct sockaddr_in my_address;
user_state_t current_user_state;
card_t *handled_card;

list_t missing_reviews;

useraddr_t *new_useraddr()
{
	useraddr_t *useraddr = malloc(sizeof(useraddr_t));
	if (!useraddr)
		goto error;

	// Inizializza i campi sicuramente noti di useraddr.
	// Da specifiche di progetto tutti i processi sono
	// in esecuzione sullo stesso host.
	useraddr->user_address.sin_family = AF_INET;
	inet_pton(AF_INET, "127.0.0.1", &useraddr->user_address.sin_addr);

	init_list(&useraddr->list);
	return useraddr;
error:
	return NULL;
}

void destroy_useraddr(useraddr_t *useraddr)
{
	if (!useraddr)
		return;

	pop_elem(&useraddr->list);
	free(useraddr);
}

void clear_useraddr_list(list_t *useraddr_list)
{
	while (!list_empty(useraddr_list))
	{
		useraddr_t *useraddr = (useraddr_t *)useraddr_list->next;
		destroy_useraddr(useraddr);
	}
}

int init_socket(uint16_t port)
{
	port = htons(port);
	init_list(&missing_reviews);
	current_user_state = STATE_IDLE;
	handled_card = NULL;

	int mysock = socket(AF_INET, SOCK_STREAM, 0);
	if (mysock == -1)
		goto error;

	// Permetti di riusare la stessa porta dopo poco tempo
	int ret = setsockopt(mysock, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int));
	setsockopt(mysock, SOL_SOCKET, SO_REUSEPORT, &(int){1}, sizeof(int));
	if (ret == -1)
		goto socket_made_error;

	my_address.sin_family = AF_INET;
	my_address.sin_addr.s_addr = INADDR_ANY;
	my_address.sin_port = port;

	ret = bind(mysock, (struct sockaddr *)&my_address, sizeof(my_address));
	if (ret == -1)
		goto socket_made_error;

	struct sockaddr_in server_addr;
	inet_pton(AF_INET, "127.0.0.1", &server_addr.sin_addr);
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(5678);

	ret = connect(mysock, (struct sockaddr *)&server_addr, sizeof(server_addr));
	if (ret == -1)
		goto socket_made_error;

	ret = sendf(mysock, "%s", command_strings[HELLO]);
	if (ret == -1)
		goto socket_made_error;

	// Aspetta conferma dal server
	command_t *command;
	recv_command(mysock, &command);
	if (!command)
		goto socket_made_error;
	command_token_t command_token = command->id;
	destroy_command(command);

	if (command_token != HELLO)
		goto socket_made_error;

	my_socket = mysock;
	return mysock;

	// Gestione dell'errore
socket_made_error:
	close(mysock);
error:
	return -1;
}

int init_listener_socket()
{

	int listener_sock = socket(AF_INET, SOCK_STREAM, 0);
	if (listener_sock == -1)
		goto error;

	int one = 1;
	int ret = setsockopt(listener_sock, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(one));
	setsockopt(listener_sock, SOL_SOCKET, SO_REUSEPORT, &(int){1}, sizeof(int));
	if (ret == -1)
		goto socket_made_error;

	ret = bind(listener_sock, (struct sockaddr *)&my_address, sizeof(my_address));
	if (ret == -1)
		goto socket_made_error;

	ret = listen(listener_sock, MAX_BACKLOG);
	if (ret == -1)
		goto socket_made_error;

	return listener_sock;

socket_made_error:
	close(listener_sock);
error:
	return -1;
}

void *review_thread_f(void *arg)
{
	int secs = rand() % 5 + 2; // Simula tempo di review casuale tra 2 e 6 secondi
	sleep(secs);
	struct sockaddr_in *useraddr = (struct sockaddr_in *)arg;
	int newsock = socket(AF_INET, SOCK_STREAM, 0);
	setsockopt(newsock, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int));


	int ret = connect(newsock, (struct sockaddr *)useraddr, sizeof(*useraddr));
	if (ret == -1)
	{
		log_line("Invio accettazione review: Errore durante la connessione.\n");
		goto end;
	}

	if (sendf(newsock, "%s accept %hu",
			  command_strings[REVIEW_CARD], ntohs(my_address.sin_port)) == -1)
	{
		log_line("Invio accettazione review: Errore durante l'invio del comando.\n");
		goto end;
	}

	log_line("[REVIEW_CARD] accept -> %hu \n", ntohs(useraddr->sin_port));
end:
	free(arg);
	close(newsock);
	return NULL;
}

int accept_request(int listener_sock)
{
	// Accetta connessione TCP
	struct sockaddr_in useraddr;
	socklen_t size_useraddr = sizeof(useraddr);
	int user_sock = accept(listener_sock, (struct sockaddr *)&useraddr, &size_useraddr);
	if (user_sock == -1)
		goto error;

	// Attendi un comando con timeout
	fd_set user_fd_set;
	FD_ZERO(&user_fd_set);
	FD_SET(user_sock, &user_fd_set);
	struct timeval timeout = _UTENTE_TIMEOUT;
	int select_return = select(user_sock + 1, &user_fd_set, NULL, NULL, &timeout);

	// Errore select o utente non ha risposto in tempo
	if (select_return == -1 || !FD_ISSET(user_sock, &user_fd_set))
		goto sock_created_error;

	command_t *command;
	recv_command(user_sock, &command);

	if (!command)
		goto sock_created_error;

	if (command->id != REVIEW_CARD) // Interazione non valida
		goto command_created_error;

	char *request_type = strtok(command->content, " ");
	if (!request_type)
		goto command_created_error;

	if (strcmp(request_type, "request") == 0) // Nuova richiesta di review ricevuta
	{
		char *port_token = strtok(NULL, " ");
		uint16_t port = (uint16_t)atoi(port_token);
		pthread_t review_thread;
		struct sockaddr_in *useraddr_copy = malloc(sizeof(struct sockaddr_in));
		memcpy(useraddr_copy, &useraddr, sizeof(struct sockaddr_in));

		useraddr_copy->sin_port = htons(port);
		pthread_create(&review_thread, NULL, review_thread_f, useraddr_copy);

		// Detach del thread poiché non è necessario sincronizzarsi con esso
		pthread_detach(review_thread); 
	}
	else if (strcmp(request_type, "accept") == 0) // Accettazione review ricevuta
	{
		char *port_token = strtok(NULL, " ");
		uint16_t port = (uint16_t)atoi(port_token);
		if (current_user_state != STATE_REVIEWING)
			goto command_created_error;
		// Rimuovi utente dalla lista di review mancanti
		for (list_t *it = missing_reviews.next; it != &missing_reviews; it = it->next)
		{
			useraddr_t *it_useraddr = (useraddr_t *)it;
			if (it_useraddr->user_address.sin_port == htons(port))
			{
				pop_elem(&it_useraddr->list);
				log_line("Review accettata da utente %hu\n", port); 
				destroy_useraddr(it_useraddr);
				break;
			}
		}
		if (list_empty(&missing_reviews))
		{
			current_user_state = STATE_DONE;
			int err = sendf(my_socket, "%s", command_strings[CARD_DONE]);
			if (err == -1)
			{
				log_line("Errore nell'invio del comando CARD_DONE alla lavagna.\n"
						 "Rirprova manualmente con il comando CARD_DONE da terminale.\n");
				goto command_created_error;
			}
			log_line("Tutte le review ricevute, inviato CARD_DONE\n");
			current_user_state = STATE_IDLE;
			destroy_card(handled_card);
			handled_card = NULL;
		}
	}
	else
	{
		log_line("Formato non valido per REVIEW_CARD\n");
	}

	close(user_sock);
	destroy_command(command);
	return 0;

command_created_error:
	destroy_command(command);

sock_created_error:
	close(listener_sock);

error:
	return -1;
}

int net_event()
{
	command_t *command;
	recv_command(my_socket, &command);
	if (!command)
		goto error;

	network_handler_t handler = network_handlers[command->id];
	if (!handler)
		goto command_created_error;
	int err = handler(command);

	destroy_command(command);
	return err;

command_created_error:
	destroy_command(command);

error:
	return -1;
}
/* ---- USER NETWORK HANDLERS ----*/

int ignore_command(command_t *command)
{
	(void)command;
	return 0;
}

int handle_QUIT(command_t *command)
{
	(void)command;
	// Termina semplicemente l'esecuzione.
	log_line("Ricevuto QUIT dalla lavagna, termino l'esecuzione...\n");
	pid_t pid = getpid();
	kill(pid, SIGINT);
	return 0;
}

int handle_SHOW_LAVAGNA(command_t *command)
{
	if (!command || !command->content)
		goto error;

	log_line(command->content);
	return 0;

error:
	return -1;
}

/**
 * @brief Invia una richiesta di review a un utente specificato da `address`.
 *
 * @returns 0 in caso di successo, -1 in caso di errore.
 */
int send_review_request(struct sockaddr_in *address)
{
	int newsock = socket(AF_INET, SOCK_STREAM, 0);
	setsockopt(newsock, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int));

	int err = connect(newsock, (struct sockaddr *)address, sizeof(*address));
	if (err == -1)
	{
		end_printing();
		goto error;
	}
	sendf(newsock, "%s request %hu",
		  command_strings[REVIEW_CARD],
		  ntohs(my_address.sin_port));
	log_line("[REVIEW_CARD] request -> %hu\n", ntohs(address->sin_port));

	close(newsock);
	return 0;
error:
	return -1;
}

int handle_SEND_USER_LIST(command_t *command)
{
	if (!command || !command->content)
		goto error;

	if (current_user_state != STATE_GETTING_USER_LIST)
		goto error;

	char *tok_state = NULL;
	char *n_users_token = strtok_r(command->content, " ", &tok_state);
	uint32_t n_users = atoi(n_users_token);
	log_line("Lista utenti ricevuta (%d elementi)\n", n_users);

	if (n_users == 0)
	{
		log_line("Nessun altro utente connesso: "
				 "Aspetta che un altro utente si connetta\n");
		current_user_state = STATE_HANDLING;
		return 0;
	}

	for (uint32_t i = 0; i < n_users; i++)
	{
		char *address_token = strtok_r(NULL, " ", &tok_state);
		if (!address_token)
			goto error;

		useraddr_t *useraddr = new_useraddr();

		if (!useraddr)
			goto error;

		// Aggiungi subito alla lista in modo che sia
		// distrutto in caso di errore (clear_useraddr_list)
		push_back(&missing_reviews, &useraddr->list);

		useraddr->user_address.sin_port = htons((uint16_t)atoi(address_token));

		if (send_review_request(&useraddr->user_address) == -1)
		{
			log_line("Errore nell'invio della richiesta di revisione\n"
					 "Potrebbe essere dovuto alla disconnessione di un utente presente sulla lista.\n");
			goto error;
		}
	}

	current_user_state = STATE_REVIEWING;
	return 0;

error:
	clear_useraddr_list(&missing_reviews);
	current_user_state = STATE_HANDLING;
	log_line("Errore: riprova la revisione manualmente con il comando REVIEW_CARD.\n");
	return -1;
}

int handle_PING_USER(command_t *command)
{
	(void)command;

	int err = sendf(my_socket, "%s ", command_strings[PONG_LAVAGNA]);
	if (err == -1)
		goto error;
	log_line("[PONG_LAVAGNA] -> lavagna\n");
	return 0;

error:
	return -1;
}

int handle_HANDLE_CARD(command_t *command)
{
	if (!command || !command->content)
		goto error;

	// Rifiuta una carta se ne sto gestendo un'altra
	if (current_user_state != STATE_IDLE)
		goto error;

	// Parsa contenuto del comando
	//  1. Numero di utenti
	char *tok_state = NULL;
	char *tok = strtok_r(command->content, " ", &tok_state);
	uint32_t n_users = atoi(tok);

	// 2. porte degli utenti
	for (uint32_t i = 0; i < n_users; i++)
	{
		tok = strtok_r(NULL, " ", &tok_state);

		// Comando malformato
		if (!tok)
			goto error;

		// Scarta lista utenti: va chiesta nuovamente
		// in fase di revisione
	}

	// Comando malformato
	if (!tok_state)
		goto error;

	handled_card = new_card(0, tok_state);
	if (!handled_card)
		goto error;

	int err = sendf(my_socket, "%s ", command_strings[ACK_CARD]);
	if (err == -1)
		goto error;

	current_user_state = STATE_HANDLING;
	log_line("[ACK_CARD] -> lavagna\n", tok_state);
	log_line("Carta ricevuta: %s\n", handled_card->desc);

	for (uint16_t percent = 33; percent < 100; percent += 33)
	{
		printf("\rGestione carta... %d%%", percent);
		fflush(stdout);
		sleep(1); // Simula tempo di gestione carta
	}

	rewrite_prompt("Utente@%hu", ntohs(my_address.sin_port));
	log_line("Carta gestita. Inizio fase di review.\n");
	cli_handlers[REVIEW_CARD](NULL);
	return 0;

error:
	destroy_card(handled_card);
	return -1;
}

network_handler_t network_handlers[N_COMMAND_TOKENS] = {
	[HELLO] = ignore_command,
	[QUIT] = handle_QUIT,
	[CREATE_CARD] = ignore_command,
	[MOVE_CARD] = ignore_command,
	[SHOW_LAVAGNA] = handle_SHOW_LAVAGNA,
	[SEND_USER_LIST] = handle_SEND_USER_LIST,
	[PING_USER] = handle_PING_USER,
	[PONG_LAVAGNA] = ignore_command,
	[HANDLE_CARD] = handle_HANDLE_CARD,
	[ACK_CARD] = ignore_command,
	[REQUEST_USER_LIST] = ignore_command,
	[REVIEW_CARD] = ignore_command,
	[CARD_DONE] = ignore_command};