#import "style/style.typ": document
#import "@preview/cetz:0.4.2"

#context{
  if (counter(page).final().at(0)>2) {
    panic("Limite pagine superato")
  }
}
#show: document.with(
  title: [Documentazione progetto (matricola dispari)],
  subtitle: [Lorenzo Mugnaioli, 677231],
)

// Previeni seprarazione di I/O e di vari acronimi
#show "I/O": box
#show regex("\b[A-Z]+\b"): box

= Istruzioni di compilazione
Nella cartella del progetto eseguire il comando
#highlight[`make all`]. Saranno prodotti due link simbolici
`./lavagna` e `./utente` nella cartella radice del progetto,
che possono essere eseguiti come descritto nelle specifiche.

Il codice è diviso in 4 cartelle:
- `/lib/` e `/include/`: contengono rispettivamente
  sorgenti e header relativi a funzioni utili sia per la
  lavagna che per l'utente
- `/utente/`: sorgenti relativi all'utente
- `/lavagna/`: sorgenti relativi alla lavagna


= Protocollo applicativo
L'applicazione descritta dalle specifiche di progetto
si tratta di un'applicazione _loss-sensitive_ (ad esempio,
le descrizioni delle card create devono essere corrette).
L'applicazione non necessita né di trasferimenti di grandi
quantità di dati né di garanzie su latenza o throughput:
non si tratta qunidi di un'applicazione _time-sensitive_.
Per questi due motivi ho quindi deciso di ricorrere al
protocollo di trasporto TCP.

== Binary contro Text
Le informazioni contenute nei messaggi sono composte
principalmente da 2 tipi di informazioni:

+ le descrizioni delle card -- scambiate ad esempio quando
  un utente richiede la creazione di una nuova _card_ o
  richiede lo stato corrente della lavagna per la
  visualizzazione sul terminale -- che sono solo testo,
  non c'è nessun vantaggio nell'utilizzare un protocollo
  binario in questo caso

+ i numeri di porta degli utenti connessi
  (`REQUEST_USER_LIST`): in questo caso un protocollo
  binario esprime qualsiasi porta in 2 byte, mentre un
  protocollo testuale può richiedere fino a 5 byte; non
  solo è maggiore, ma è anche a lunghezza variabile,
  quindi è richiesto un carattere aggiuntivo per segnalare
  la fine di una porta e l'inizio della successiva!

Alla fine ciò che ha condizionato maggiormente la scelta
è stato il punto 1 e ho scelto di implementare un
protocollo di tipo _text_.  Questa scelta ha inoltre semplificato
notevolmente le fasi di debug durante lo sviluppo del
progetto, essendo il contenuto dei messaggi facilmente
leggibile da Wireshark.

Dalle specifiche di progetto il
numero di byte da leggere deve essere comunicato: il
contenuto vero e proprio è preceduto da due byte di
lunghezza, espressa in binario, come mostrato in @msg-format.

#figure(caption: [Formato dei messaggi, lunghezze riportate
  sopra i campi in byte.])[
  #cetz.canvas({
    import cetz.draw: *
    scale(75%)

    rect((), (rel: (3, 1)), name: "length")
    content((rel: (0, 0.4), to: "length.north"))[2]
    content("length")[Lunghezza]

    rect("length.north-east", (rel: (6, -1)), name: "content")
    content((rel: (0, 0.4), to: "content.north"))[Lunghezza]
    content("content")[Contenuto]
  })
]<msg-format>


= Struttura dell'applicazione
== Lavagna
La _lavagna_ deve gestire uno stato che può
essere modificato da tante parti (i vari utenti).
Le due possibili decisioni progettuali che sono
possibili sono:
+ Lavagna multi-threaded: richiede di implementare accesso
  in mutua esclusione alle variabili di stato

+ Lavagna single-threaded: richiede di implementare
  multiplexing I/O tra i vari utenti.

Ho optato per la seconda, dato che il tempo di interazione
effettivo tra lavagna e utenti è una piccola frazione del
tempo in cui i programmi rimangono in esecuzione ; inoltre
ritengo che l'implementazione con multiplexing I/O risulti
in codice più facilmente mantenibile rispetto ad
un'implementazione con accesso in mutua esclusione.

La gestione dei timeout è stata implementata con un
metodo di polling, ovvero periodicamente vengono
controllati i timeout associati agli utenti connessi
per poi essere opportunamente gestiti. Per evitare attese
attive si è fatto ricorso alla chiamata `alarm()` e
alla definizione di un handler per `SIGALRM`.

Per evitare l'accesso alle strutture dati in momenti
inopportuni da parte dell'handler (ad esempio mentre
è in esecuzione la routine di gestione di una richiesta
di un utente, che può a sua volta accedere alle strutture
sopracitate), quest'ultimo si limita a segnalare il
passaggio del periodo di polling scrivendo in una pipe,
il cui _file descriptor_ dell'estremità di lettura è
compreso nel meccanismo di multiplexing.



== Utente
L'utente si tratta di un'entità non banale pur essendo
più semplice della lavagna: non deve gestire uno stato
comune modificato da più attori ma deve supportare
la comunicazione con gli altri utenti e mantenere un suo
stato interno (@user-fsm).

#figure(caption: [State Machine dell'utente (escluse fasi
  di connessione e di disconnessione)])[

  #set text(size: 7pt)
  #show text: it => align(center)[#it]
  #show math.equation: it => align(center)[#it]
  #set par(spacing: 0em)
  #let dline = line
  #cetz.canvas({
    import cetz.draw: *
    scale(80%)
    circle((), radius: 1, name: "state-idle")
    content("state-idle")[IDLE]

    circle((rel: (5, 0), to: "state-idle"), radius: 1, name: "state-handling")
    content("state-handling")[HANDLING\ CARD]

    circle((rel: (1, -3.5), to: "state-handling"), radius: 1, name: "state-getting-user-list")
    content("state-getting-user-list")[GETTING\ USER\ LIST]

    anchor("state-done", (rel: (-1, -3.5), to: "state-idle"))

    circle((rel: (0, -2), to: ("state-getting-user-list", 50%, "state-done")), radius: 1, name: "state-reviewing")
    content("state-reviewing")[REVIEWING]

    circle("state-done", radius: 1, name: "state-done")
    content("state-done")[DONE]

    set-style(mark: (end: "straight"))

    let start = "state-idle.30deg"
    let end = "state-handling.150deg"
    bezier(start, end, (rel: (0, 1), to: (start, 50%, end)), name: "transition-1")
    content((rel: (0, 0.2), to: "transition-1.50%"))[#box(fill: white, outset: 2pt)[
      carta ricevuta\ #dline(end: (100%, 0%))\ invio ACK CARD]]

    start = "state-handling.-35deg"
    end = "state-getting-user-list.72deg"
    bezier(start, end, (rel: (0.5, 0.25), to: (start, 50%, end)), name: "transition-2")
    content((rel:(-0.5,0.1), to:"transition-2.60%"))[#box(fill: white, outset: 2pt)[
      richiesta review\ #dline(end: (100%, 0%))\ invio REQUEST USER LIST]]

    start = "state-getting-user-list.-105deg"
    end = "state-reviewing.0deg"
    bezier(start, end, (rel: (.5, -0.5), to: (start, 50%, end)), name: "transition-3")
    content((rel: (0.8, -0.1), to: "transition-3.50%"))[
      #box(
        fill: white,
        outset: 2pt,
      )[ricezione lista utenti\ #dline(end: (100%, 0%))\ invio REVIEW CARD\ agli altri utenti]]

    start = "state-reviewing.180deg"
    end = "state-done.-79deg"
    bezier(start, end, (rel: (-0.4, -0.5), to: (start, 50%, end)), name: "transition-4")
    content((rel: (-0.9, -0.1), to: "transition-4.50%"))[
      #box(fill: white, outset: 2pt)[tutte le review ricevute\ #dline(length: 100%)\ $Lambda$]]

    start = "state-done.109deg"
    end = "state-idle.-144deg"
    bezier(start, end, (rel: (-0.4, 0.0), to: (start, 50%, end)), name: "transition-5")
    content((rel: (-0.4, -0.1), to: "transition-5.50%"))[
      #box(fill: white, outset: 2pt)[ $Lambda$\ #dline(length: 100%)\ invio CARD DONE ]]

    end = "state-idle.135deg"
    start = (rel: (-1, 1), to: end)
    line(start, end, stroke: (dash: (3pt, 1.5pt)))
  })
]<user-fsm>

Dato che in qualunque momento un utente deve poter
richiedere la review, un socket dell'utente associato
alla stessa porta con cui esso comunica con la lavagna
rimane in ascolto #footnote[Ottenere questo comportamento
  richiede di impostare ad 1 `SO_REUSEADDR` e `SO_REUSEPORT`
  tramite la funzione `setsockopt()`.].

Qui sorge un problema: un utente deve poter mandare
richieste a tanti utenti e ricevere risposte dopo qualche
secondo; non è pensabile aspettare la risposta di una
richiesta prima di inviare la successiva. Di nuovo le
opzioni sono 2:
+ Connessioni non persistenti: ovvero usare due connesisoni
  TCP distinte per la richiesta e per la risposta
+ Implementare un sistema di multiplexing I/O simile a
  quello della lavagna per le connessioni attive tra gli
  utenti

Essendo l'applicazione elastica dal punto di vista della
banda, ho ritenuto l'overhead comportato dall'apertura di
più connessioni TCP meno gravoso della complicazione del
codice utente dovuta all'implementazione di un sistema di
multiplexing #footnote[Un meccanismo di multiplexing è
  presente nel programma relativo all'utente, ma i file
  descriptor sono fissati all'inizio dell'esecuzione].

Le connssioni non persistenti garantiscono una maggiore
flessibilità nell'interazione tra gli utenti:
è possibile intercettare sia richieste di review sia
risposte a richieste di review prestando attenzione solo
al socket su cui l'utente rimane in ascolto, dato che
entrambe queste operazioni richiedono l'apertura di una
nuova connessione (non devo trasferire la gestione di una
connessione già aperta tra il thread principale e il nuovo
thread).

Infine, dato che l'operazione di risposta ad una richiesta
di revisione è un'operazione completamente sconnessa
rispetto al resto delle operazioni svolte da un utente,
viene generato un nuovo thread che per prima cosa
invoca `sleep()`, per poi rispondere aprendo e chiudendo
una connessione all'interno della funzione che il thread
va ad eseguire.

= Esempio
Vorrei concludere la documentazione andando ad esporre
il funzionamento del protocollo applicativo mostrando ciò
che avviene durante la fase di revisione, che è la fase
più complessa che un utente attraversa.
Negli schemi sottostanti verranno esclusi i primi 2 byte
di lunghezza.

== Richiesta lista utenti
#figure(placement: none, caption: [Richiesta lista utenti e formato
  messaggi. I rettangoli vuoti rappresentano spazi e tutti i numeri
  (compresi i numeri di porta) sono scritti testualmente.])[
  #set box(fill: white, outset: 0.2em)
  #grid()[
    #set text(size: 9pt)
    #cetz.canvas({
      import cetz.draw: *

      content((), name: "utente")[Utente]
      content((rel: (5, 0), to: "utente"), name: "lavagna")[Lavagna]
      set-style(mark: (end: "straight"))
      line((rel: (0, -0.2), to: "utente.south"), (rel: (0, -2.2)))
      line((rel: (0, -0.2), to: "lavagna.south"), (rel: (0, -2.2)))
      //Primo messaggio
      anchor("u-1", (rel: (0, -0.5), to: "utente.south"))
      anchor("l-1", (rel: (0, -1), to: "lavagna.south"))
      line("u-1", "l-1", name: "m-1")
      content("m-1.50%")[#box[`REQUEST_USER_LIST`]]

      anchor("l-2", (rel: (0, 0), to: "l-1"))
      anchor("u-2", (rel: (-5, -.5), to: "l-1"))
      line("l-2", "u-2", name: "m-2")
      content("m-2.50%")[#box[`SEND_USER_LIST`]]
    })
  ][
    #v(0.3cm)
    #set text(size: 9pt)
    #cetz.canvas({
      import cetz.draw: *
      rect((), (rel: (4, -0.5)), name: "cmd")
      content("cmd")[`REQUEST_USER_LIST`]

      rect((rel: (0, -1), to: "cmd.north-west"), (rel: (3.3, -0.5)), name: "cmd")
      content("cmd")[`SEND_USER_LIST`]

      rect("cmd.north-east", (rel: (0.2, -0.5)), name: "space")
      rect("space.north-east", (rel: (0.5, -0.5)), name: "n")
      content("n")[_n_]

      rect("n.north-east", (rel: (0.2, -0.5)), name: "space")
      rect("space.north-east", (rel: (1, -0.5)), name: "last")
      content("last")[_port 1_]

      rect("last.north-east", (rel: (0.2, -0.5)), name: "space")
      rect("space.north-east", (rel: (1, -0.5)), name: "last")
      content("last")[_port 2_]

      rect("last.north-east", (rel: (0.6, -0.5)), name: "space", stroke: (dash: "dashed"))
      rect("space.north-east", (rel: (1, -0.5)), name: "last")
      content("last")[_port n_]
    })
  ]
]

== Richiesta review

#figure(placement: none, caption: [Formato del messaggio di richeista review
  e approvazione review])[
  #set text(size: 9pt)
  #cetz.canvas({
    import cetz.draw: *
    rect((), (rel: (2.8, -0.5)), name: "cmd")
    content("cmd")[`REVIEW_CARD`]
    rect("cmd.north-east", (rel: (0.2, -0.5)), name: "space")
    rect("space.north-east", (rel: (1.5, -0.5)), name: "arg")
    content("arg")[`request`]
    rect((rel: (0, -1), to: "cmd.north-west"), (rel: (2.8, -0.5)), name: "cmd")
    content("cmd")[`REVIEW_CARD`]
    rect("cmd.north-east", (rel: (0.2, -0.5)), name: "space")
    rect("space.north-east", (rel: (1.5, -0.5)), name: "arg")
    content("arg")[`accept`]
  })
]

== Passaggio finale
#figure(placement: none, caption: [Formato del messaggio inviato alla
  lavagna dopo la ricezione di tutte le approvazioni])[
  #set text(size: 9pt)
  #cetz.canvas({
    import cetz.draw: *
    rect((), (rel: (2.5, -0.5)), name: "cmd")
    content("cmd")[`CARD_DONE`]
  })
]
