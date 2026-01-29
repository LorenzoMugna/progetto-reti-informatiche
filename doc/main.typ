#import "style/style.typ": document

#show: document.with(
	title: [Documentazione progetto],
	subtitle: [Lorenzo Mugnaioli]
)

= Istruzioni di compilazione
Nella cartella del progetto eseguire il comando
#highlight[`make all`]. Saranno prodotti due file
`./lavagna` e `./utente` nella cartella radice del progetto.


= Protocollo applicativo
== Binary contro Text
L'applicazione non necessita né di trasferimenti di grandi
quantità di dati né di garanzie su latenza o throughput.

I messaggi scambiati tra gli host dell applicazione
risultano piuttosto semplici: , bensì
di utilizzare un protocollo applicativo di tipo *text*,
che risulta più facilmente modificabile e integrabile.

Di nuovo, non essendo critica la performance ma dovendo
scambiare dati in maniera affidabile tra gli host
coinvolti, il protocollo di trasporto su cui si basa
l'applicazione sara il *TCP*, andando a risparmiarci
l'implementazione di un protocollo RDT _ad-hoc_.

= Struttura dell'applicazione
== Lavagna
La _lavagna_ deve rispondere ad una serie di eventi, tra cui
la connessione di nuovi utenti, a cui la lavagna risponde
inviando una card da gestire (`HANDLE_CARD`) e i timeout
(`PING_USER`, `PONG_LAVAGNA`).

Per questo motivo le varie componenti della lavagna sono
state organizzate su thread diversi e si riconoscono 3
componenti, ognuno atto a gestire un'aspetto diverso:
- Il gestore di stato
- Il gestore di rete
- Il gestore di timeout

Il *gestore di stato* si occupa di mantenere aggiornato lo
stato (card e colonne di appartenenza) in risposta ai
comandi che riceve da una coda, che viene popolata dal
*gestore di rete*.

Il *gestore di rete* si occupa di popolare la coda con le
richieste provenienti dagli utenti (`HELLO`, `QUIT`,
`CREATE_CARD`) e riceve i comandi da inviare agli utenti
dagli altri gestori.



#colbreak(weak: true)
#lorem(100)

#lorem(100)
#pagebreak()

= #lorem(2)
#lorem(100)