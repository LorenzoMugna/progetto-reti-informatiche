#import "style/style.typ": document

#show: document.with(
	title: [Documentazione progetto],
	subtitle: [Lorenzo Mugnaioli]
)

=== Text contro binary
L'applicazione non necessita di trasferimenti
di grandi quantità di dati [...].

Considerato questo e in previsione di
-- ipotetici -- futuri aggiornamenti e
addizioni di funzionalità, è stato deciso
di non scambiare messaggi binari tra gli host
dell'applicazione, bensì di utilizzare un
protocollo applicativo *text-based*.

#colbreak(weak: true)
#lorem(100)

#lorem(100)