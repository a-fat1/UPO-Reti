# UniUPO - Reti (1)
In questa reposity sono presenti delle coppie di programmi in C, Client e Server, che si scambiano messaggi tra di loro attraverso socket TCP o UDP. La cartella ["Esercizi"](https://github.com/a-fat1/UniUPO-Reti/tree/main/Esercizi) contiene le (mie) soluzioni dei compiti assegnati durante le lezioni di laboratorio del corso di Reti, mentre la cartella ["Progetti"](https://github.com/a-fat1/UniUPO-Reti/tree/main/Progetti) contiene delle coppie di Client-Server molto più articolate rispetto agli esercizi di laboratorio.

## Informazioni utili

I programmi presenti nella cartella esercizi, che cominciano con il codice 01, sono la base sviluppata dal docente.
Gli altri esercizi (e i progetti), invece, sono quelli che sono stati assegnati e svolti per il laboratorio.

## Compilare i sorgenti

Per compilare e generare gli eseguibili dei programmi si consiglia di utilizzare il seguente comando da terminale bash:

```
gcc <nome_file>.c -Wall -Wextra -std=c11 -pedantic -g -o <nome_file>
```

## Eseguire i programmi

Per eseguire correttamente i programmi bisogna prima avviare il Server, e successivamente si può far partire il Client.

- Per avviare il Server da terminale:

```
./<nome_file> <porta>
```

- Per avviare il Client da terminale:

```
./<nome_file> <indirizzo_ip> <porta>
```
