# Finire la base
- finire il filesystem FAT32 - Almost done
- aggiungere il supporto agli ELF
- aggiungere i timer
- aggiungere i processi

# Oltre la base
## Roba a basso livello più complicata
- supportare lo swapping
- usare più di un core
    scheduling
    multiprocessing
    multithreading
- networking

## Alzare il livello di astrazione
- progettare un modello di astrazione (e.g. su Unix "everything is a file")
- creare un'api ispirata al modello di astrazione che abbiamo scelto

> I programmi devono interagire con il kernel senza conoscere i dettagli di come è implementato. Possiamo anche astrarre poco, e rendere disponibili nell'api funzioni specifiche per ogni hardware specifico.

# Creare un parco software
- scrivere una libreria che consenta di usare l'api del kernel dal C (equivalente di unistd.h per unix)
- scrivere la libreria standard del C wrappando la nostra api (consente di fare porting di software su butos)
- scrivere una shell - ancora da rendere indipendente
- scrivere un editor di testo
- scrivere un assembler

# Configurare la compilazione
- creare uno script di configurazione