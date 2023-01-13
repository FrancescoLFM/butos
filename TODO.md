# Finire la base
- finire il filesystem FAT32
- aggiungere il supporto agli ELF
- aggiungere i timer
- aggiungere i processi
- aggiustare il bootloader perché funzioni su hardware vero
- potenziare il boot loader in due fasi
    - abilitare la a20 line
    - fare memory mapping
    - caricare il kernel nella memoria dedicata

# Oltre la base
## Roba a basso livello più complicata
- passare al paging
- supportare la memoria virtuale
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
- scrivere una shell
- scrivere un editor di testo
- scrivere un assembler
