#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h>
#include "../utility/utility.h"

#define MAX 50

int main(int argc, char *argv[]){

	//Dichiarazione struttura dell'indirizzo del Server	
	struct sockaddr_in my_client;	

	//Dichiarazione del Socket descriptor
	int skclient;

	//Dichiarazione variabile per la scelta dell'Host
	int scelta;

	//Dichiarazione variabile usata nei cicli
	int i = 0;
	
	//Dichiarazione del buffer ricettore della lista degli Host
	char buffClient[200]; 

	//Dichiarazione del buffer di backup per lo spacchettamento del buffer
	char buffBackup[200];

	//Dichiarazione del buffer ricettore della risposta del Server (informazioni dell'Host)
	char buffRispostaServer[200];

	//Dichiarazione stringa usata per spacchettare il buffBackup
	char *temp = NULL;
	
	//Dichiarazione stringa per la conversione dell'indice Host a stringa
	char num[4];

	//Dichiarazione stringa per la lettura della scelta
	char strscelta[4];
	
	//Dichiarazione array di Host
	char **host;	
	
	
	//Inizializzazione array di Host
	host = (char**)malloc(MAX * sizeof(char*));
	for(i = 0; i < MAX; i++){
		host[i] = (char*)malloc(MAX * sizeof(char));
		strcpy(host[i], " ");
	}
	i = 0;
	
	
	//**************************************************************
	//************** Inizio connessione con il Server **************
	//**************************************************************


	//Inizializzo la struttura dell'indirizzo del Server
	my_client.sin_family = AF_INET;
	my_client.sin_port = htons(atoi(argv[2])); //argv[2] è il secondo valore dato da linea di comando (porta del Server)
	inet_aton(argv[1], &my_client.sin_addr); //argv[1] è il primo valore dato da linea di comando (indirizzo IP del Server)

					
	do{
		
		//Inizializzazione di buffClient
		memset(buffClient, 0, sizeof(buffClient));
	
		//Inizializzazione di buffBackup
		memset(buffBackup, 0, sizeof(buffBackup));

		//Inizializzazione di buffRispostaServer
		memset(buffRispostaServer, 0, sizeof(buffRispostaServer));
	

		//Creo un Socket TCP
		skclient = socket(PF_INET, SOCK_STREAM, 0);

		//Connetto il Client al Server e controllo che la connessione avvenga con successo
		if (connect(skclient, (struct sockaddr*)&my_client, sizeof(my_client)) < 0){
			printErrorToMonitor("ERRORE: la connessione è fallita\n");
			exit(-1);	
		}

		//Scrivo sul Socket una stringa vuota per simulare la richiesta della lista degli Host dal Server 
		write(skclient, " ", 1);	
		
		//Leggo dal Socket la lista degli Host appena ricevuta dal Server
		read(skclient, buffClient, sizeof(buffClient));
		
		//Eseguo una copia di buffClient su buffBackup
		strcpy(buffBackup, buffClient);
		
		//Chiudo momentaneamente la connessione con il Server
		close(skclient);

		i = 0;
		
		//Inizio a spacchettare buffBackup per reperire tutti i nomi degli Host
		temp = strtok(buffBackup, " ");
		while(temp != NULL){
			strcpy(host[i], temp);
			temp = strtok(NULL, " ");
			i++;
		}
		
		i = 0;

		//***************************************************
		//* Inizio stampa del menù per la scelta di un Host *
		//***************************************************

		//Controllo se la prima posizione dell'array Host contiene una stringa vuota
		if ( strlen(host[i]) > 1){ //La prima posizione dell'array Host contiene una stringa non vuota (il nome di un Host)

			printToMonitor("========== MENU HOST ==========\n");
				
			while( (strlen(host[i]) > 1) && i < MAX){
					
				//Converto l'indice intero in stringa per la stampa
				sprintf(num, "%d", i + 1);

				printToMonitor(num);
				printToMonitor(" - ");
				printToMonitor(host[i]);
				printToMonitor("\n");
				i++;
			}
			printToMonitor("0 - Exit\n\n");
			
			//Leggo da standard input la scelta fatta dall'utente
			printToMonitor("\nSeleziona un HOST tra i presenti nella lista: ");
			read(STDIN_FILENO, strscelta, sizeof(strscelta));

			//Converto la stringa in un intero
			scelta = atoi(strscelta);
				
		//***************************************************
		//** Fine stampa del menù per la scelta di un Host **
		//***************************************************

		}else{
			printErrorToMonitor("ERRORE: non ci sono host del quale richiedere informazioni\n");
			scelta = 0;
		}


		//Controllo se la scelta è compresa tra 1 e il numero di Host che ho in array
		if(scelta > 0 && scelta <= i){
			//Il controllo della scelta è andato a buon fine

			printToMonitor("Stai per ricevere le informazioni di questo host -> ");
			printToMonitor(host[scelta - 1]);
			printToMonitor("\n");
			
			//Mi ricollego al Server				
			skclient = socket(PF_INET, SOCK_STREAM, 0);
			if (connect(skclient, (struct sockaddr*)&my_client, sizeof(my_client)) < 0){
				printErrorToMonitor("ERRORE: la connessione è fallita\n");
				exit(-1);	
			}
		
			//Scrivo sul Socket il nome dell'Host scelto
			write(skclient, host[scelta - 1], strlen(host[scelta - 1]));
			
			//Leggo dal Socket le informazioni riguardanti quel dato Host
			read(skclient, buffRispostaServer, sizeof(buffRispostaServer));

			//Chiudo il Socket				
			close(skclient);

			//Stampo le informazioni dell'Host
			printToMonitor("DATI RICEVUTI: ");
			printToMonitor(buffRispostaServer);	
			printToMonitor("\n\n");

		}else if(scelta != 0)
			//Il controllo della scelta non è andato a buon fine e stampo un errore
			printErrorToMonitor("ERRORE: non è stato selezionato un host valido. Riprova!\n");

	}while(scelta > 0 && scelta < MAX);
	

	//**************************************************************
	//*************** Fine connessione con il Server ***************
	//**************************************************************
}
