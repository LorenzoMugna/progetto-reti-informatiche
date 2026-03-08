#import "style/style.typ": document
#import "@preview/cetz:0.4.2"

#context {
  if (counter(page).final().at(0) > 2) {
    panic("Limite pagine superato")
  }
}
#show: document.with(
  title: [Documentazione progetto (matricola dispari)],
  subtitle: [Lorenzo Mugnaioli, 677231],
)

// Previeni separazione di I/O e di vari acronimi
#show "I/O": box
#show regex("\b[A-Z]+\b"): box

= Istruzioni di compilazione
Il codice è suddiviso in 4 cartelle:
- `src/lib/` e `src/include/`: contengono rispettivamente
  sorgenti e header relativi a funzioni utili sia per la
  lavagna che per l'utente
- `src/utente/`: sorgenti e header relativi all'utente
- `src/lavagna/`: sorgenti e header relativi alla lavagna

Nella stessa cartella in cui è presente il file `Makefile`,
eseguire il comando #highlight[`make all`]. Al termine
della compilazione saranno prodotti due link simbolici,
`./lavagna` e `./utente`, nella cartella radice del progetto,
che possono essere eseguiti come descritto nelle specifiche.

*Cambiare la dimensione della finestra mentre i programmi 
sono in esecuzione potrebbe causare problemi di
visualizzazione.  Si consiglia di posizionare prima le
finestre e poi mandare in esecuzione i programmi.*


= Protocollo applicativo
L'applicazione descritta nelle specifiche di progetto
è *loss-sensitive*. Ad esempio, le descrizioni delle card
create devono essere trasmesse senza errori. L'applicazione non
necessita di garanzie su latenza o throughput
particolarmente stringenti: non si tratta quindi di
un'applicazione *time-sensitive*.

Per questi due motivi ho quindi deciso di ricorrere al
protocollo di trasporto TCP.

== Binary contro Text
Le informazioni contenute nei messaggi sono composte
per la maggior parte da stringhe di testo di lunghezza
variabile, con l'unica eccezione delle liste di
porte inviate nei messaggi scambiati nelle implementazioni
di `HANDLE_CARD` e `SEND_USER_LIST`.

Ho quindi optato per un protocollo di tipo *text*, con
lo svantaggio di dover creare una logica di parsing dei
messaggi in arrivo.

I vantaggi di questa scelta si sono resi evidenti
nell'immediatezza con cui vengono costruiti i messaggi,
non dovendo pensare all'endianness e costruendo un
messaggio in maniera analoga all'utilizzo di una
`printf()`.
Ritengo che la fase di debug sia stata notevolmente
semplificata come conseguenza di questa scelta, essendo il
contenuto dei messaggi facilmente leggibile da Wireshark.

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
essere modificato da diverse parti (i vari utenti).
Inoltre, le interazioni lavagna-utente sono molto brevi e
sparse su grandi intervalli di tempo.

Considerato quanto sopra, ho optato per mantenere la
lavagna single-threaded, ritenendo un singolo thread
sufficiente a gestire le richieste provenienti dai vari
utenti e facilitando così il mantenimento della
consistenza delle varie strutture dati.

In compenso, ho dovuto implementare un meccanismo di
multiplexing I/O che supporti una quantità di socket variabile
nel tempo: a tale scopo ho utilizzato la funzione `poll()`,
più adatta per questo specifico _use-case_ rispetto alla
`select()`; quest'ultima richiede la ricostruzione del
`fd_set` ad ogni utilizzo.

La gestione dei timeout è stata implementata tramite
polling. Per evitare attese attive si è fatto ricorso alla
chiamata `alarm()` e alla definizione di un handler per
`SIGALRM`.

Per evitare l'accesso alle strutture dati comuni
in maniera concorrente, l'handler si limita a segnalare il
passaggio del periodo di polling scrivendo in una pipe,
il cui _file descriptor_ dell'estremità di lettura è
compreso nel meccanismo di multiplexing di cui sopra.

=== Strutture dati
Per effettuare facilmente aggiunte e rimozioni di card e
utenti dai vari elenchi, le tre colonne (To Do, Doing,
Done) della lavagna e la lista degli utenti sono
implementate con liste doppiamente concatenate.
Alla lista degli utenti è affiancata una tabella di
corrispondenza tra numeri di porta e descrittori degli
utenti, in modo da accedere velocemente ad un utente con
porta nota.

== Utente
L'utente è un'entità non banale pur essendo
più semplice della lavagna: non deve gestire uno stato
comune modificato da più attori ma deve supportare
la comunicazione con gli altri utenti e mantenere un suo
stato interno (@user-fsm).

#figure(caption: [State Machine dell'utente (escluse fasi
  di connessione e di disconnessione)])[

  #set text(size: 6.95pt)
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
    content((rel: (-0.5, 0.1), to: "transition-2.60%"))[#box(fill: white, outset: 2pt)[
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

    start = "state-reviewing.110deg"
    end = "state-handling.-160deg"
    bezier(start, end, (rel: (-1, 1.0), to: (start, 50%, end)), name: "transition-5")
    content((rel: (-0.1, -1), to: "transition-5.50%"))[
      #box(fill: white, outset: 2pt)[ Errore invio REVIEW CARD \ #dline(length: 100%)\ $Lambda$]]

    end = "state-idle.135deg"
    start = (rel: (-1, 1), to: end)
    line(start, end, stroke: (thickness: 0.7pt, dash: (3pt, 1pt)))
  })
]<user-fsm>

Dato che in qualunque momento un utente potrebbe ricevere
richieste di review, un socket dell'utente associato
alla stessa porta con cui esso comunica con la lavagna
rimane in ascolto (`listen()`)#footnote[
  Ottenere questo comportamento
  richiede di impostare ad 1 `SO_REUSEADDR` e `SO_REUSEPORT`
  tramite la funzione `setsockopt()`.
].

Si pone quindi il problema: un utente deve poter mandare
richieste di revisione a (possibilmente) tanti utenti e
ricevere risposte dopo qualche secondo; non è pensabile
aspettare la risposta di una richiesta prima di inviare la
successiva.

Una possibile soluzione è utilizzare due connessioni TCP
distinte per la richiesta di revisione e la risposta.
Questa soluzione ha lo svantaggio di avere un overhead per
l'apertura di più connessioni ma presenta il vantaggio di
poter gestire in maniera indipendente la richiesta e la
risposta: un utente può mandare tante richieste aprendo e
chiudendo le connessioni per poi ricevere le risposte
prestando attenzione solo al socket di ascolto.

Dato che l'operazione di risposta ad una richiesta
di revisione è un'operazione completamente sconnessa
rispetto al resto delle operazioni svolte da un utente,
viene generato un nuovo thread che risponde dopo un
intervallo di tempo casuale.

= Esempio interazione
Vorrei concludere la documentazione andando ad esporre
il funzionamento del protocollo applicativo mostrando ciò
che avviene durante la fase di revisione, probabilmente la
più complessa che un utente attraversa.
Negli schemi sottostanti sono esclusi i primi 2 byte
di lunghezza.

== Richiesta della lista degli utenti
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

#figure(placement: none, caption: [Formato del messaggio di richiesta review
  e approvazione review. Ogni freccia rappresenta una diversa connessione.])[
  #set text(size: 9pt)
  #set box(fill: white, outset: 0.2em)
  #grid()[
    #cetz.canvas({
      import cetz.draw: *
      content((), name: "utente")[Utente]
      content((rel: (5, 0), to: "utente"), name: "u1")[Altri utenti]
      anchor("u1", (rel: (-0.3, 0), to: "u1.south"))
      anchor("u2", (rel: (0.3, 0)))
      anchor("u3", (rel: (0.3, 0)))
      set-style(mark: (end: "straight"))
      line((rel: (0, -0.2), to: "utente.south"), (rel: (0, -4.5)))
      line((rel: (0, -0.2), to: "u1"), (rel: (0, -4.5)))
      line((rel: (0, -0.2), to: "u2"), (rel: (0, -4.5)))
      line((rel: (0, -0.2), to: "u3"), (rel: (0, -4.5)))

      line((rel: (0, -0.5), to: "utente.south"), (rel: (0, -1), to: "u1", update: false))
      line((rel: (0, -0.3)), (rel: (0, -1.3), to: "u2", update: false), name: "requests")
      line((rel: (0, -0.3)), (rel: (0, -1.6), to: "u3", update: false))
      content("requests.50%")[#box[Richieste]]

      line((rel: (0, -2.5), to: "u1"), (rel: (-4.7, -1), update: false))
      line((rel: (0, -2.6), to: "u3"), (rel: (-5.3, -1.17), update: false), name: "responses")
      line((rel: (0, -2.9), to: "u2"), (rel: (-5, -1.15), update: false))
      content("responses.50%")[#box[Risposte]]
    })
  ][
    #v(0.3cm)
    #cetz.canvas({
      import cetz.draw: *
      rect((), (rel: (2.8, -0.5)), name: "cmd")
      content("cmd")[`REVIEW_CARD`]
      rect("cmd.north-east", (rel: (0.2, -0.5)), name: "space")
      rect("space.north-east", (rel: (1.5, -0.5)), name: "arg")
      content("arg")[`request`]
      rect("arg.north-east", (rel: (0.2, -0.5)), name: "space")
      rect("space.north-east", (rel: (1.5, -0.5)), name: "arg")
      content("arg")[_src port_]

      rect((rel: (0, -1), to: "cmd.north-west"), (rel: (2.8, -0.5)), name: "cmd")
      content("cmd")[`REVIEW_CARD`]
      rect("cmd.north-east", (rel: (0.2, -0.5)), name: "space")
      rect("space.north-east", (rel: (1.5, -0.5)), name: "arg")
      content("arg")[`accept`]
      rect("arg.north-east", (rel: (0.2, -0.5)), name: "space")
      rect("space.north-east", (rel: (1.5, -0.5)), name: "arg")
      content("arg")[_src port_]
    })
  ]
]
Nei messaggi di richiesta e approvazione review è
contenuta la porta dell'utente che ha inviato il messaggio.
Questo perché riassegnare la stessa porta ripetutamente
causa problemi con l'esecuzione del programma quando si va
ad aumentare il numero degli utenti. Si lascia quindi
assegnare una porta al sistema operativo.

== Passaggio finale
#figure(placement: none, caption: [Formato del messaggio inviato alla
  lavagna dopo la ricezione di tutte le approvazioni])[
  #set text(size: 9pt)
  #grid()[
    #set text(size: 9pt)
    #set box(fill: white, outset: 0.2em)
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
      content("m-1.50%")[#box[`CARD_DONE`]]
    })

  ][
    #v(0.3cm)
    #cetz.canvas({
      import cetz.draw: *
      rect((), (rel: (2.5, -0.5)), name: "cmd")
      content("cmd")[`CARD_DONE`]
    })
  ]
]
