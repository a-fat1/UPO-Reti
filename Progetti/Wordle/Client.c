#include <ctype.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>


int writemex(int cmex, int simpleSocket);
int readmex(int cmex, int simpleSocket);
int checkmex(int cmex, char buffer[], int returnStatus);
//Nuova funzione: mexcontainer

int main(int argc, char* argv[]) 
{
    int simpleSocket = 0, simplePort = 0, returnStatus = 0, cmex, endsignal;
    struct sockaddr_in simpleServer;

    if (argc == 3)
    {
        simpleSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

        if (simpleSocket != -1)
        {
            simplePort = atoi(argv[2]);

            memset(&simpleServer, '\0', sizeof(simpleServer));
            simpleServer.sin_family = AF_INET;
            simpleServer.sin_addr.s_addr = inet_addr(argv[1]);
            simpleServer.sin_port = htons(simplePort);

            returnStatus = connect(simpleSocket, (struct sockaddr*)&simpleServer, sizeof(simpleServer));

            if (returnStatus == 0)
            {
                cmex = 0;       //Quando cmex = 0 il client si aspetta il messaggio di benvenuto 
                endsignal = 0;

                while(endsignal != 1)
                {
                    if(cmex > 0)
                        endsignal = writemex(cmex, simpleSocket);       //Controllo e invio al server
                    if(endsignal != 1)
                        endsignal = readmex(cmex, simpleSocket);        //Legge, analizza e stampa a schermo

                    cmex++;
                }

                printf("\n");
            }
            else
            {
                fprintf(stderr, "\n\nErrore: impossibile connettersi all'indirizzo IP.\n\n\n");
                close(simpleSocket);
                exit(1);
            }
        }
        else
        {
            fprintf(stderr, "\n\nErrore: creazione socket fallita.\n\n\n");
            exit(1);
        } 
    }
    else
    {
        fprintf(stderr, "\n\nPer avviare il programma sono necessari 3 argomenti: <nome programma> <IPv4 server> <porta server>\n\n<nome programma> equivale a: %s\n<IPv4 server> equivale all'indirizzo IP del programma server a cui ci si vuole collegare (Es. 81.22.36.245)\n<porta server> corrisponde alla porta del server che possiede un valore numerico compreso tra 1024 e 65535\n\nGli argomenti, inoltre, devono essere separati fra di loro con degli spazi.\n\n\n", argv[0]);
        exit(1);
    }
    
    return 0;
}


int writemex(int cmex, int simpleSocket)
{
    char buffer[260], word[6];
    int returnStatus, checkchar, posword, endsignal = 0, repeat = 1;

    do
    {
        fprintf(stdout, "\nTentativo numero %d\n", cmex);
        scanf("%s", word);

        //Controllo se word = fine

        if (((word[0] == 'F' || word[0] == 'f') && (word[1] == 'I' || word[1] == 'i') && (word[2] == 'N' || word[2] == 'n') && (word[3] == 'E' || word[3] == 'e')) && word[4] == '\0')
        {
            sprintf(buffer, "QUIT\n");
            repeat = 0;
        }
        else
        {
            //Controllo numero di caratteri inserito

            if(strlen(word) != 5)
                fprintf(stdout, "\n\nErrore, la parola deve contenere 5 caratteri. Inseriscila di nuovo.\n\n");
            else
            {
                checkchar = 0;
                posword = 0;

                while(posword<5 && checkchar == 0)      //Ciclo per controllare i caratteri alfabetici
                {
                    if(isalpha(word[posword]) == 0)
                    {
                        fprintf(stdout, "\n\nErrore, la parola deve contenere solo caratteri alfabetici. Inseriscila di nuovo.\n\n");
                        checkchar = 1;
                    }
                    else
                        posword++;
                }

                if(checkchar == 0)
                {
                    sprintf(buffer, "WORD %s\n", word);
                    repeat = 0;
                }
            }
        }
    }
    while (repeat == 1);

    returnStatus = write(simpleSocket, buffer, strlen(buffer));     //Invia il messaggio al server

    if (returnStatus <= 0)      //Controllo la connessione con il server
    {
        fprintf(stderr, "\n\nErrore: invio messaggio fallito, impossibile stabilire una connessione con il server.\n\n");
        endsignal = 1;
    }

    return endsignal;
}


int readmex(int cmex, int simpleSocket)
{
    int returnStatus, endsignal;
    char buffer[260];

    returnStatus = read(simpleSocket, buffer, sizeof(buffer));      //Legge messaggio dal server

    if (returnStatus > 0)
    {
        if (returnStatus <= 256)        //Controllo limite caratteri nel messaggio ricevuto
            endsignal = checkmex(cmex, buffer, returnStatus);       //Identifica il tipo di messaggio e stampa il contenuto
        else
        {
            fprintf(stderr, "\n\nErrore: il messaggio ricevuto supera il limite massimo di 256 caratteri.\n\n");
            endsignal = 1;
        }                 
    }
    else
    {
        fprintf(stderr, "\n\nErrore: impossibile stabilire una connessione con il server.\n\n");
        endsignal = 1;
    }

    return endsignal;
}


int checkmex(int cmex, char buffer[], int returnStatus)
{
    int posbuffer = 0, cspace = 0, endsignal = 0;
    char alphanumber[3];

    if(cmex == 0)       //Controlli su messaggio di benvenuto (struttura messaggio: OK X(X) <mex>\n)
    {
        if(buffer[0] == 'O' && buffer[1] == 'K')        //Conferma protocollo OK
        {
            if(buffer[2] == ' ')        //Spazio separatore tra protocollo e tentativi
            {
                if(buffer[returnStatus - 1] == '\n')        //Invio finale
                {
                    if(isdigit(buffer[3]) > 0)      //Controllo carattere tentativi = cifra
                    {
                        cspace = 1;

                        if(isdigit(buffer[4]) > 0)      //Controllo se il numero di tentativi possiede due cifre
                        {
                            if(buffer[5] != ' ')        //Spazio separatore tra tentativi e testo messaggio
                                cspace = 0;
                            else
                            {
                                //Salvo le due cifre dei tentativi in una stringa da convertire 

                                alphanumber[0] = buffer[3];
                                alphanumber[1] = buffer[4];
                                alphanumber[2] = '\0';
                            }
                        }
                        else
                        {
                            if(buffer[4] != ' ')        //Spazio separatore tra tentativi e testo messaggio
                                cspace = 0;
                            else
                            {
                                //Salvo la cifra dei tentativi in una stringa da convertire

                                alphanumber[0] = buffer[3];
                                alphanumber[1] = '\0';
                                alphanumber[2] = '\0';
                            }
                        }

                        if(cspace == 1)
                        {
                            if(atoi(alphanumber) >= 6 && atoi(alphanumber) <= 10)       //Controllo numero tentativi (numero di tentativi accettabili: tra 6 e 10)
                            {
                                printf("\n\n");
                                posbuffer = 0;
                                cspace = 0;
    
                                while(posbuffer < returnStatus)     //Ciclo per stampare il messaggio di benvenuto, conta gli spazi per saltare protocollo e numero di tentativi
                                {
                                    if(cspace < 2)
                                    {
                                        if(buffer[posbuffer] == ' ')
                                            cspace++;
                                    }
                                    else
                                        printf("%c", buffer[posbuffer]);

                                    posbuffer++;
                                }

                                fprintf(stdout, "\n\nRegole del gioco:\n- Puoi scrivere solo una parola di 5 caratteri alfabetici;\n- '*' lettera corretta, '+' lettera presente ma in posizione errata, '-' lettera assente;\n- Il gioco finisce se esaurisci i tentativi.\n\nBuona fortuna!\n\nTentativi totali: %d.\nSe desideri arrenderti digita la parola 'fine'.\nOppure premi i pulsanti CTRL+C per terminare immediatamente il programma.\n\n", atoi(alphanumber));

                            }
                            else
                            {
                                fprintf(stderr, "\n\nErrore: questo client accetta dal server un numero di tentativi compreso tra 6 e 10.\n\n");
                                endsignal = 1;
                            }
                        }
                        else
                        {
                            fprintf(stderr, "\n\nErrore: assenza del carattere spazio tra tentativi totali e testo del messaggio.\n\n");
                            endsignal = 1;
                        }
                    }
                    else
                    {
                        fprintf(stderr, "\n\nErrore: impossibile identificare il numero dei tentativi totali.\n\n");
                        endsignal = 1;
                    }
                }
                else
                {
                    fprintf(stderr, "\n\nErrore: carattere d'invio finale assente.\n\n");
                    endsignal = 1;
                }
            }
            else
            {
                fprintf(stderr, "\n\nErrore: assenza del carattere spazio tra protocollo e tentativi totali.\n\n");
                endsignal = 1;
            }        
        }
        else
        {
            fprintf(stderr, "\n\nErrore: impossibile riconoscere il protocollo del messaggio di benvenuto.\n\n");
            endsignal = 1;
        }
    }
    else
    {
        if(buffer[0] == 'O' && buffer[1] == 'K')        //Conferma protocollo OK (struttura messaggio: OK X(X) YYYYY\n o OK PERFECT\n)
        {
            if(buffer[2] == ' ')        //Spazio separatore tra protocollo e tentativi
            {
                if(buffer[returnStatus - 1] == '\n')        //Invio finale
                {
                    if(returnStatus == 11 || returnStatus == 12)        //Controllo dimensione del messaggio
                    {
                        if(returnStatus == 11)      //Se la dimensione del messaggio = 11 bisogna verificare la presenza di 'PERFECT' nel messaggio
                        {
                            if(buffer[3] == 'P' && buffer[4] == 'E' && buffer[5] == 'R' && buffer[6] == 'F' && buffer[7] == 'E' && buffer[8] == 'C' && buffer[9] == 'T')
                            {
                                fprintf(stdout, "\n\nComplimenti, hai indovinato la parola!\n\n");
                                endsignal = 1;
                            }
                        }

                        if(endsignal == 0)
                        {
                            if(isdigit(buffer[3]) > 0)      //Controllo carattere tentativi = cifra
                            {
                                if(returnStatus == 12)      //Dimensione del messaggio: 12-11 caratteri, in base al numero di cifre dei tentativi
                                {
                                    if(isdigit(buffer[4]) <= 0 || buffer[5] != ' ')     //Controllo se il numero di tentativi possiede due cifre + spazio separatore tra tentativi e testo messaggio
                                    {
                                        fprintf(stderr, "\n\nErrore: impossibile identificare lo spazio separatore e/o il numero di tentativo attuale del messaggio.\n\n");
                                        endsignal = 1;
                                    }
                                    else
                                    {
                                        //Salvo le due cifre dei tentativi in una stringa da convertire

                                        alphanumber[0] = buffer[3];
                                        alphanumber[1] = buffer[4];
                                        alphanumber[2] = '\0';
                                    }
                                }
                                else
                                {
                                    if(buffer[4] != ' ')        //Spazio separatore tra tentativi e testo messaggio
                                    {
                                        fprintf(stderr, "\n\nErrore: assenza del carattere spazio tra tentativo attuale e stringa del messaggio.\n\n");
                                        endsignal = 1;
                                    }
                                    else
                                    {
                                        //Salvo la cifra dei tentativi in una stringa da convertire

                                        alphanumber[0] = buffer[3];
                                        alphanumber[1] = '\0';
                                        alphanumber[2] = '\0';
                                    }
                                }
                            }
                            else
                                fprintf(stderr, "\n\nErrore: impossibile identificare il numero del tentativo attuale.\n\n");

                            if(endsignal == 0)
                            {
                                if(cmex == atoi(alphanumber))       //Controllo numero tentativo attuale tra client e server
                                {
                                    posbuffer = 2;

                                    while(posbuffer <= 6 && endsignal == 0)     //Ciclo per controllare che la stringa del messaggio abbia solo i caratteri '-', '+' o '*'
                                    {
                                        if(buffer[returnStatus - posbuffer] == '-' || buffer[returnStatus - posbuffer] == '+' || buffer[returnStatus - posbuffer] == '*')
                                            posbuffer++;
                                        else
                                            endsignal = 1; 
                                    }

                                    posbuffer = 0;

                                    if(endsignal == 0)
                                        posbuffer = 3;
                                    else
                                        fprintf(stderr, "\n\nErrore: presenza di caratteri non consentiti nella stringa di simboli.\n\n");
                                }
                                else
                                    fprintf(stderr, "\n\nErrore: tentativo attuale del messaggio e del client non corrispondenti.\n\n");
                            }
                        }
                    }
                    else
                    {
                        fprintf(stderr, "\n\nErrore: il messaggio non rispetta il numero di caratteri prestabilito.\n\n");
                        endsignal = 1;
                    }
                }
                else
                {
                    fprintf(stderr, "\n\nErrore: carattere d'invio finale assente.\n\n");
                    endsignal = 1;
                }
            }
            else
            {
                fprintf(stderr, "\n\nErrore: assenza del carattere spazio tra protocollo e tentativo attuale.\n\n");
                endsignal = 1;
            }
        }
        else
        {
            if((buffer[0] == 'E' && buffer[1] == 'N' && buffer[2] == 'D') || (buffer[0] == 'E' && buffer[1] == 'R' && buffer[2] == 'R'))        //Conferma protocollo END o ERR (END X(X) YYYYY\n o ERR <mex>)
            {
                if(buffer[3] == ' ')        //Controllo presenza spazio separatore
                {
                    if(buffer[returnStatus - 1] == '\n')        //Invio finale
                    {
                        if(buffer[0] == 'E' && buffer[1] == 'R' && buffer[2] == 'R')        //In caso di errore, stampa solo il messaggio
                        {
                            posbuffer = 4;
                            printf("\n\n");
                        }
                        else
                        {
                        
                            if(returnStatus == 12 || returnStatus == 13)        //Controllo dimensione del messaggio
                            {
                                if(isdigit(buffer[4]) > 0)      //Controllo carattere tentativi = cifra
                                {
                                    if(returnStatus == 13)      //Dimensione del messaggio: 13-12 caratteri, in base al numero di cifre dei tentativi
                                    {
                                        if(isdigit(buffer[5]) <= 0 || buffer[6] != ' ')     //Controllo se il numero di tentativi possiede due cifre + spazio separatore tra tentativi e testo messaggio
                                        {
                                            fprintf(stderr, "\n\nErrore: impossibile identificare lo spazio separatore e/o il numero di tentativo attuale del messaggio.\n\n");
                                            endsignal = 1;
                                        }
                                        else
                                        {
                                            //Salvo le due cifre dei tentativi in una stringa da convertire

                                            alphanumber[0] = buffer[4];
                                            alphanumber[1] = buffer[5];
                                            alphanumber[2] = '\0';
                                        }
                                    }
                                    else
                                    {
                                        if(buffer[5] != ' ')        //Spazio separatore tra tentativi e testo messaggio
                                        {
                                            fprintf(stderr, "\n\nErrore: assenza del carattere spazio tra tentativo attuale e stringa del messaggio.\n\n");
                                            endsignal = 1;
                                        }
                                        else
                                        {
                                            //Salvo la cifra dei tentativi in una stringa da convertire

                                            alphanumber[0] = buffer[4];
                                            alphanumber[1] = '\0';
                                            alphanumber[2] = '\0';
                                        }
                                    }
                                }
                                else
                                    fprintf(stderr, "\n\nErrore: impossibile identificare il numero del tentativo attuale.\n\n");

                                if(endsignal == 0)
                                {
                                    if(cmex == atoi(alphanumber))       //Controllo numero tentativo attuale tra client e server
                                    {   
                                        posbuffer = 2;

                                        while(posbuffer <= 6 && endsignal == 0)       //Ciclo per controllare che la stringa del messaggio abbia solo i caratteri alfabetici
                                        {
                                            if(isalpha(buffer[returnStatus - posbuffer]) > 0)
                                                posbuffer++;
                                            else
                                                endsignal = 1; 
                                        }

                                        posbuffer = 0;

                                        if(endsignal == 0)
                                        {
                                            posbuffer = 4;
                                            printf("\n\nGioco terminato, la parola era ");
                                        }   
                                        else
                                            fprintf(stderr, "\n\nErrore: la parola deve contenere solo caratteri alfabetici.\n\n");
                                    }
                                    else
                                        fprintf(stderr, "\n\nErrore: tentativo attuale del messaggio e del client non corrispondenti.\n\n");
                                }
                            }
                        }
                    }
                    else
                        fprintf(stderr, "\n\nErrore: carattere d'invio finale assente.\n\n");
                }
                else
                    fprintf(stderr, "\n\nErrore: assenza del carattere spazio tra protocollo e tentativo attuale.\n\n");
            }
            else
            {
                if(buffer[0] == 'Q' && buffer[1] == 'U' && buffer[2] == 'I' && buffer[3] == 'T')        //Conferma protocollo QUIT (QUIT <mex>\n)
                {
                    if(buffer[4] == ' ')        //Spazio separatore tra protocollo e messaggio
                    {
                        if(buffer[returnStatus - 1] == '\n')        //Invio finale
                        {
                            posbuffer = 5;
                            printf("\n\n");
                        }
                        else
                            fprintf(stderr, "\n\nErrore: carattere d'invio finale assente.\n\n");
                    }
                    else
                        fprintf(stderr, "\n\nErrore: assenza del carattere spazio tra protocollo e testo del messaggio.\n\n"); 
                }
                else
                    fprintf(stderr, "\n\nErrore: impossibile identificare il protocollo del messaggio.\n\n");
            }

            endsignal = 1;
        }

        if(posbuffer == 3 || posbuffer == 4 || posbuffer == 5)
        {
            while (posbuffer < returnStatus)        //Ciclo per stampare i caratteri del messaggio, senza protocollo
            {
                if(buffer[0] == 'E' && buffer[1] == 'N' && buffer[2] == 'D')        //If per stampare punto finale tra parola e invio se i tentativi sono esauriti
                    if(posbuffer == returnStatus - 1)
                        printf(".");

                if((buffer[0] == 'E' && buffer[1] == 'R' && buffer[2] == 'R') || (buffer[0] == 'Q' && buffer[1] == 'U' && buffer[2] == 'I' && buffer[3] == 'T'))        //Stampa il contenuto di QUIT e ERR
                    printf("%c", buffer[posbuffer]);
                else
                {
                    //Stampa il contenuto dei protocolli OK ed END
                    if (cspace >= 1)
                        printf("%c", buffer[posbuffer]);

                    if (buffer[posbuffer] == ' ')
                        cspace++;
                }

                posbuffer++;
            }

            printf("\n");
        }
    }
    
    return endsignal;
}