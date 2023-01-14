# Iniziare con butos
Scrivo questa breve dispensa in italiano, in modo tale da essere il più chiaro
possibile con i nuovi potenziali collaboratori della repository. Iniziamo dal
principio. Butos è un software che è fatto per eseguire prima che parta il
vostro sistema operativo. Di fatti butos viene chiamato direttamente dal BIOS, e
una volta che il BIOS lo esegue ha il pieno controllo della vostra macchina.
Terrificante, ma in realtà non è necessario eseguirlo sul "nudo metallo".
Vediamo allora quali strumenti servono per compilare, eseguire e lavorare con
butos.

## Strumenti necessari
- gcc: comprende programmi come `as`, `ld` (usati per assemblare il bootloader)
    e lo stesso `gcc` che non ha bisogno di introduzioni;
- qemu-system-x86_64: serve ad emulare butos in modo tale da non doverlo testare
    su hardware;
- make: serve ad automatizzare il processo di compilazione di butos

Inoltre a questa lista si aggiungono altri strumenti consigliati per lo
sviluppo:
- gdb: serve a svolgere operazioni di debugging. Purtroppo il debugging di un
    kernel è più rognoso del debugging di un normale software in userspace. Per
    questa ragione consigliamo di testare prima in userspace le eventuali nuove
    funzionalità che volete aggiungere a butos - ovviamente quando possibile;
- scan-build: analizzatore statico del progetto `llvm`, molto utile per trovare
    bug prima di eseguire il codice

## Hello World
Andiamo al dunque. Siamo tutti programmatori qua dentro: quando %rip salta sul
nostro codice c'è poco da fare, quel codice ha un sapore tutto diverso. Allora
va bene, vi spiego come scrivere un programma minimale per iniziare a sviluppare
con butos.

Entrate nella cartella `src/`. È qui infatti che si trova tutto il codice C di
butos. Le altre cartelle non sono importanti al momento, e magari le
approfondiremo in seguito. Comunque nel `README.md` trovate qualcosa. Una volta
che vi trovate in `src/` create il file `main.c` e scrivete:

```c
/**
 * File: main.c
 * 
 * Description: Simple "Hello World" program for butos
 */

#include <libs/print.h>

int main()
{
    puts("Hello World\n");
    return 0;
}
```

Inserite il prototipo della funzione main nel file `startk.c`. Qui vive la
funzione `_start()`, che è l'entry point di butos. È qui che inizia il control
flow del programma. Togliete l'eventuale funzione di testing che si trova prima
dell'istruzione `stop()` e chiamate il vostro `main()`.

A questo punto salvate tutto, tornate nella cartella principale del progetto
(prima di `src/`) e provate a compilare con `make`. Se `gcc` non dà errori
allora eseguite butos con `make run`. Questa regola lancerà un'istanza di
`qemu-system-x86_64` e avvierà automaticamente butos. A questo punto, se avete
fatto tutto correttamente, vedrete la stringa comparire sullo schermo.

## Qualcosa in più
Ora che vedete e credete possiamo iniziare a introdurre un po' la struttura
della codebase di butos. Adesso stiamo ancora giocando, ma pian piano inizieremo
a diventare più seri.

All'interno della cartella `src/` troviamo varie cartelle. Esse classificano i
file sorgente `.c` e gli header `.h` in base allo scopo che svolgono. Dividiamo
i file sorgente in due macro-categorie:
- librerie: sono quelle simpatiche perché richiedono poche conoscenze tecniche
    di come funziona il kernel. In realtà il loro scopo è principalmente quello
    di servire i driver. Ad esempio qui troviamo le funzioni di i/o, varie
    implementazioni di strutture dati che possono servire e un allocatore. In
    genere una nuova libreria si aggiunge quando serve al resto del kernel.
    Insomma, non troverete l'intera libc, però riconoscerete molte funzioni cui
    siete abituati. Noi in questa dispensa ci concentreremo sulle funzioni di
    libreria perché è il posto giusto da cui partire.
- driver: implementano funzioni di comunicazione diretta con l'hardware.
    Li troviamo nelle cartelle `driver/` e `cpu/` in base all'hardware con cui
    vanno a interagire. Gli interrupt ad esempio sono pertinenza della cpu, per
    cui li troviamo nella seconda cartella, mentre i driver ata per leggere il
    disco sono nella prima cartella.

Le librerie che intendo introdurre in questa dispensa sono quelle base per
scrivere dei programmi in grado di fare qualcosa. Lasciatemi mettere un attimo
avanti le mani: siamo in ambiente bare metal, qui non avete la libc e altre
comodità che vi ha dato casa GNU. Qui siete voi e il freddo metallo. Spesso vi
troverete a implementare voi stessi le funzioni che vi servono perché non le
troverete scritte da nessun'altra parte nel progetto. Altrettanto spesso vi
troverete a implementare funzioni che già esistono. Vi consiglio di conoscere le
funzioni base su cui potete contare in modo tale da non reinventare la ruota
ogni volta, ma anche di non scoraggiarvi se vedete che non c'è la vostra
funzione del cuore.

## Le librerie fondamentali
Ecco qui una lista delle librerie che intendo coprire in questa dispensa:
- print
- scan
- string
- ctype
- alloc

Per poter utilizzare queste librerie utilizzate:
```c
#include <libs/nome.h>
```
Dove `nome` è il nome della libreria.

### print
La libreria `print` contiene le funzioni di stampa a video. Le principali
funzioni che vi serviranno sono tre:
```c
int putc(int c);
int puts(const char* string);
int printk(char *str, ...);
```

- `putc()`: Stampa il carattere `c` a video.
- `puts()`: Stampa la stringa `string` a video. Attenzione! Non va a capo come la
    puts della libreria standard del C.
- `printk()`: È una versione non-standard e limitata della printf. Supporta gli
    operatori di formato:
    - `%d`: intero con segno (decimale)
    - `%u`: intero senza segno (decimale)
    - `%o`: intero senza segno (ottale)
    - `%x`: intero senza segno (esadecimale)
    - `%b`: intero senza segno (binario)
    - `%s`: stringa
    Attenzione! Non avete il supporto ai modificatori di formato di nessun tipo.
    Non avete i flag, il padding né niente del genere, ma se volete stampare un
    numero potete farlo con questa funzione.

Inoltre insieme a queste tre funzione ci sono le alternative omonime che
terminano in `_c` per stampare con un colore. Usate la sintassi
`BACKGROUND(COLOR) | FOREGROUND(COLOR)` per impostare il colore del background e
del foreground. Quello che non specificate verrà sottinteso come nero.

### scan
La libreria `scan` contiene le funzioni di input da tastiera. Inoltre qui sono
comprese le funzioni di conversioe da stringa a intero, che può essere utile
avere insieme alle funzioni di input.

Al momento in cui sto scrivendo questa dispensa la `scanf` ancora non esiste,
per cui dovrete faticare un po' di più. Potete sempre implementarla voi :). Le
funzioni di cui di dispone questa libreria sono:

```c
int getchar();
char *gets(char *s, size_t size);
long strtol(const char *nptr, char **endptr, int base);
```

Un po' di teoria: ogni volta che battete un tasto lanciate un interrupt al
vostro sistema operativo che lo deve gestire. Butos lo gestisce leggendo l'input
da tastiera e convertendolo in un comando (ES: Caps Lock è un comando che
modifica lo stato di capitalizzazione del testo che battete) oppure in un
carattere. I caratteri vengono messi in una coda (implementata nella libreria
`queue`.

- `getchar()` estrae un carattere dalla coda di input, lo stampa a video e lo
    restituisce al chiamante. Se la coda è vuota, aspetta che si riempia.
    Insomma, vi lascia tutto il tempo che volete per battere un tasto.
- `gets()` serve a leggere comodamente una stringa. Diversamente dalla funzione
    standard del C (che è altamente deprecata) richiede un limite massimo oltre
    il quale si rifiuta di scrivere. Nel comportamento, quindi, è più simile
    alla `fgets()` standard. Ogni stringa letta è sempre terminata con `\0`.
- `strtol()` è la funzione standard che converte una stringa in un `long int`

### string
Su `string` c'è poco da dire. Cerca di implementare un gran numero di funzioni
per lavorare agevolmente con la memoria (`mem-`) e con le stringe (`str-`). Il
comportamento di queste funzioni è pienamente ispirato allo standard. Alcune
delle vostre preferite potrebbero non essere ancora implementate, ma ne dubito.

### ctype
Anche `ctype` è interamente ispirata allo standard. Ovviamente non c'è un
`locale`, ma a parte questo `ctype` funziona come ve l'aspettereste. Se volete
chiarimenti, insomma, avete le man pages di linux.

### alloc
Torniamo fuori dallo standard con `alloc`. Questa libreria implementa le
funzioni necessarie per allocare dinamicamente memoria a runtime in modo sicuro.
In realtà potreste besissimo fare una cosa del genere:

```c
int *a = (int *)0x200000;
```

Però se vi serve che vi spieghi perché è una pessima idea siete nel posto
sbagliato. L'allocatore non è dei più efficienti visti in giro, ma fa il suo
sporco lavoro. L'algoritmo è demenziale: cerca di darvi il primo spazio libero
che trova, e se non lo trova o se è saturo vi restituisce un bel `NULL` come
avrebbe fatto la cara vecchia malloc.

Vediamo che funzioni abbiamo per allocare:

```c
void *kalloc(size_t size);
void kfree(void *ptr);
void *kcalloc(size_t nmemb, size_t size);
void *krealloc(void *ptr, size_t array);
```

- `kalloc()`: Funziona in maniera simile alla `malloc()`.
- `kfree()`: Funziona in maniera simile alla `free()`. Per aiutare nel debug
    stampa un messaggio a video e ferma l'esecuzione quando fallisce.
- `kcalloc()`: Funziona in maniera simile alla `calloc()`.
- `krealloc()`: Vediamo se sapete usare l'induzione -_-

Benissimo, abbiamo un tour generale. Divertitevi con questi giocattoli, e se vi
va di collaborare sulle parti del kernel che toccano il metallo be', vi servirà
altro, ma nel caso potete sempre contattarmi su telegram oppure fare un po' di
ricerche da voi. La TODO list può guidarvi. Cercherò di scrivere altre dispense
in ogni caso per agevolare ulteriormente chi voglia conoscere più
approfonditamente la struttura del progetto.
