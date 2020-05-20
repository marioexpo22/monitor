#define _XOPEN_SOURCE
#define __USE_XOPEN
#define _GNU_SOURCE

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
#include <time.h>
#include "../utility/utility.h"

#define MAX 50

int main(int argc, char *argv[]){

	//Dichiarazione struttura dell'indirizzo del Server
	struct sockaddr_in my_address;

	//Dichiarazione del Socket descriptor
	int sk1;

	//Dichiarazione descrittore della connessione
	int con;

	//Dichiarazione del buffer ricettore dei dati
	char buffDati[200];

	//Dichiarazione del buffer utilizzato per mandare dati al Client
	char buffDatiClient[200];

	//Dichiarazione variabile time_t del tempo locale del Server per controllo dello stato degli Host 
	time_t tempoServer;

	//Dichiarazione struttura tm per il tempo dell'Host fornito dall'Agent
	struct tm tempoAgent;
	
	//Dichiarazione variabile time_t del tempo dell'Host fornito dall'Agent per controllo dello stato degli Host
	time_t tempoAgentNormale;

	//Dichiarazione struttura DatiHost (per immagazzinare le informazioni da poter fornire al Client)
	struct DatiHost dati_agent[MAX];

	//Struttura DatiHost temporanea che salva i dati che riceve dall'AGENT
	struct DatiHost datiTemp;

	//Dichiarazione stringa usata per spacchettare il buffDati
	char *nomeTemp;
	
	//Dichiarazione variabile usata nei cicli
	int i=0;
	
	//Inizializzazione struttura DatiHost
	for(i=0; i<MAX; i++){
		strcpy(dati_agent[i].name_host, " ");
		strcpy(dati_agent[i].date, " ");
		strcpy(dati_agent[i].buffDati, " ");
		dati_agent[i].status = 0;
	}

	//**************************************************************************
	//* Inizio apertura del Socket per la connessione degli Agent e dei Client *
	//**************************************************************************

	//Inizializzo la struttura dell' indirizzo del Server
	my_address.sin_family = AF_INET;
	
	//Controllo se ho specificato su riga di comando la porta sul quale connettersi
	if(argc > 1)
		my_address.sin_port = htons(atoi(argv[1]));
	else
		//Altrimenti ne assegno una di default
		my_address.sin_port = htons(8080);

	my_address.sin_addr.s_addr = htonl(INADDR_ANY);

	//Creo un socket TCP
	sk1 = socket(PF_INET, SOCK_STREAM, 0);

	//Controllo se il Socket è stato creato correttamente
	if(sk1 < 0){
		printErrorToMonitor("ERRORE: il socket non è stato aperto correttamente\n");
		exit(-1);
	}

	//Assegno l'indirizzo del Server al Socket e controllo che avvenga con successo
	if(bind(sk1, (struct sockaddr*)&my_address, sizeof(my_address)) != 0){
		printErrorToMonitor("ERRORE: indirizzo di SK1 non assegnato correttamente\n");
		exit(-1);
	}

	//Metto il Socket in ascolto e controllo che avvenga con successo
	if(listen(sk1, 5) != 0){
		printErrorToMonitor("ERRORE: il servizio non è raggiungibile\n");
		exit(-1);	
	}
	//************************************************************************
	//* Fine apertura del Socket per la connessione degli Agent e dei Client *
	//************************************************************************
	
	//**********************************************
	//*** Inizio Server in ascolto su Socket sk1 ***
	//**********************************************
	while(1){

		//Reset di buffDati, in modo tale da poterci riscrivere nuove informazioni
		memset(buffDati, 0, sizeof(buffDati));
		
		//Reset di buffDatiClient, in modo tale da poterci riscrivere nuove informazioni
		memset(buffDatiClient, 0, sizeof(buffDatiClient));

		//Connessione sul socket degli Agent e dei Client
		con = accept(sk1, NULL, NULL);

		//Controlla se la connessione è andata a buon fine
		if(con < 0){
			printErrorToMonitor("ERRORE: il server non riesce ad accettare la connessione\n");
			exit(-1);		
		}
		
		//**********************************************
		//********** Inizio gestione dei dati **********
		//**********************************************

		//Lettura dal Socket
		read(con, &buffDati, sizeof(buffDati));		
		
		//Controllo se ho ricevuto dall'Agent o dal Client
		if(strlen(buffDati) > 50){
			//Ho ricevuto dati dall'Agent

			//******************************************************************
			//* Riempiamo la struttura datiTemp con i dati ricevuti dall'Agent *
			//******************************************************************

			//Salviamo il buffDati ricevuto prima di spacchettarlo
			strcpy(datiTemp.buffDati, buffDati);

			//Estraiamo il nome dell'Host dal buffDati
			nomeTemp = strtok(buffDati, ",");

			//Assegno a datiTemp.name_host il nome dell'Host
			strcpy(datiTemp.name_host, nomeTemp);

			//Continuo a leggere tutto il buffDati
			while(nomeTemp != NULL){
				nomeTemp = strtok(NULL, ",");
				if(nomeTemp != NULL)
					//Assegno a datiTemp.date la data di ricezione dei dati dall'Agent
					strcpy(datiTemp.date, nomeTemp);
			}
		
			//Essendo che ho ricevuto informazioni di quell'Host lo considero connesso
			datiTemp.status = 1; 

			//Assegno i dati della struttura temporanea al mio array di DatiHost
			i = 0;
			while (i < MAX){
				if ( (strcmp(dati_agent[i].name_host, datiTemp.name_host) == 0) || (strcmp(dati_agent[i].name_host, " ") == 0) || (dati_agent[i].status == 0) ){
					strcpy(dati_agent[i].name_host, datiTemp.name_host);
					strcpy(dati_agent[i].date, datiTemp.date);
					strcpy(dati_agent[i].buffDati, datiTemp.buffDati);
					dati_agent[i].status = datiTemp.status;

					i = MAX;		
				}
				i++;		
			}
		}else{	
			//Ho ricevuto dati dal Client
						
			i = 0;

			//Controllo se il Client chiede la lista degli Host o le informazioni di un determinato Host
			if(strlen(buffDati) == 1){
				//Il Client chiede la lista degli Host

				//Creo la lista di nomi di Host e la scrivo nel buffDatiClient							
				while(i < MAX){
					if(strcmp(dati_agent[i].name_host, " ") != 0){
						strcat(buffDatiClient, dati_agent[i].name_host);
						strcat(buffDatiClient, " ");
					}
					i++;
				}

				//Scrivo sul Socket la lista degli Host
				write(con, buffDatiClient, sizeof(buffDatiClient));
			}else{
				//Il Client chiede informazioni riguardo un dato Host
			
				//Controllo di quale Host il Client voglia le informazioni
				while(i < MAX){
					if (strcmp(buffDati, dati_agent[i].name_host) == 0){
						if(dati_agent[i].status == 1){
							//Se l'Host è connesso invio al Client tutte le informazioni
							strcpy(buffDatiClient, "Connesso, ");
							strcat(buffDatiClient, dati_agent[i].buffDati);												
						}else{
							//Se l'Host è disconnesso invio al Client la data dell'ultimo aggiornamento delle informazioni
							strcpy(buffDatiClient, "Disconnesso, ");
							strcat(buffDatiClient, dati_agent[i].date);
						}
						i = MAX;					
					}
					
					i++;

				}

				//Scrivo sul Socket le informazioni dell'Host
				write(con, buffDatiClient, sizeof(buffDatiClient));
			}

		}
		
		//**********************************************
		//*********** Fine gestione dei dati ***********
		//**********************************************	
		
		

		//*********************************************
		//*** Inizio controllo Host ancora connessi ***
		//*********************************************

		i = 0;
		while (i < MAX){
			if((strcmp(dati_agent[i].name_host, " ") != 0)){
				//Converto la data di ricezione nella variabile tempoAgent 			
				strptime(dati_agent[i].date, "%02H:%02M:%02S %02d/%02m/%Y", &tempoAgent);
				tempoAgentNormale = mktime(&tempoAgent);

				//Ricavo il tempo locale del Server
				time(&tempoServer);

				//Controllo se la differenza di tempo è superiore ai 6 secondi					
				if(difftime(tempoServer, tempoAgentNormale) > 6){
					//Se è superiore, imposto lo stato a 0 (disconnesso)
					dati_agent[i].status = 0;			
				}else{
					//Se è inferiore o uguale, imposto lo stato a 1 (connesso)
					dati_agent[i].status = 1;
				}
			}
			i++;
		}

		//*********************************************
		//*** Fine controllo Host ancora connessi ***
		//*********************************************
	
		//Chiusura della connessione
		close(con);
	}

	//**********************************************
	//**** Fine Server in ascolto su Socket sk1 ****
	//**********************************************

	//Chiusura Socket
	close(sk1);
}

