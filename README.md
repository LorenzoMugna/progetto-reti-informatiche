# Progetto Reti Informatiche

## Riassunto requisiti Progetto

Lavagna Kanban (porta 5678), $\ge$ 4 utenti identificati da porte (incrementali dalla 5679), $\ge$ 10 card
```
+--------------------------------------------------------+
|                  Lavagna - *ID lavagna*                |
+------------------+------------------+------------------+
|       To Do      |       Doing      |       Done       |
+------------------+------------------+------------------+
|     *task id*    |     *task id*    |     *task id*    |
|*descrizione task*|*descrizione task*|*descrizione task*|
+------------------+------------------+------------------+
|     *task id*    |     *task id*    |     *task id*    |
|*descrizione task*|*descrizione task*|*descrizione task*|
+------------------+------------------+------------------+
```

**Lavagna**
- ID
- Colonne
- Card

**Card**
- ID
- Colonna (di appartenenza)
- Descrizione Attività
- Utente che l'ha presa in carico o terminata

**Interazioni da implementare**
- `HELLO`
- `QUIT`
- `CREATE_CARD`
- `MOVE_CARD`
- `SHOW_LAVAGNA`: Mostra la lavagna ad ogni spostamento di card
- `SEND_USER_LIST`
- `PING_USER`
- `PONG_LAVAGNA`

- `HANDLE_CARD`
- `ACK_CARD`
- `REQUEST_USER_LIST`
- `REVIEW_CARD`
- `CARD_DONE`

## Schema funzionamento

**Lavagna**

- `./lavagna`: la lavagna parte sulla porta `5678`
- `HELLO` da parte di un utente: porta registrata e aggiunte a struttura dati e aggiornato contatore
- QUIT: logout utente.
- `ACK_CARD` o `CARD_DONE` da utenti: `MOVE_CARD` (doing o done)
- Timeout: se una carta è in doing da tanto tempo (e.g. 1m30s), invia `PING_USER` e se non riceve `PONG_LAVAGNA` entro 30s rimetti la carta in To Do.
- Connessione di un utente: manda card `HANDLE_CARD` (prevista ricezione `ACK_CARD`)

**Utente**
Da quello che ho capito completamente deterministico (non c'è veramente un utente)

0. Connessione `HELLO`
1. Riceve carta da (`HANDLE_CARD`) lavagna invia `ACK_CARD`
2. attendi tempo, richiedi lista (`REQUEST_USER_LIST`)
3. invia review (`REVIEW_CARD`)
    - alla ricezione di una review, aspetta un po' e poi accettala
4. ricevute tutte le review, invia alla lavagna (`CARD_DONE`)


# TODO
~~Refactor~~

Cleanliness delle interazioni (la lavagna deve essere mostrata nuovamente ad ogni interazione etc. etc.)
- Semplifica invio porte (escludi indirizzo)

Crea helpstring e viualizzala nello startup

Documentazione (quasi finita)

Lavagna:
- ~~gestione errori send (fai una common-net per primitive safe)~~
- ~~Multiplexing I/O fatto bene~~
- ~~poll dei timeout~~
- ~~gestione comandi da terminale (cli-handlers)~~
    - ~~`QUIT`~~
    - ~~`CREATE_CARD`~~
    - ~~`MOVE_CARD`~~
    - ~~`PING_USER`~~

- visualizzazione della lavagna in colonne
    - quasi fatto: manca l'id della lavagna
- Aggiungi carte di default alla lavagna

Client:
- ~~Un client non deve poter uscire se sta facendo una review~~
- ~~gestione comandi da terminale (cli-handlers)~~
- ~~gestione richieste di review~~
- Vai in review automaticamente
- Robustezza: implementazione timeout sull'invio e la ricevuta di approvazioni (non lo farò)

