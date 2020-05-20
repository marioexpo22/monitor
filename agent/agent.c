#include <stdlib.h>
#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/sysinfo.h>
#include <unistd.h>
#include <netdb.h>
#include <time.h>
#include <string.h>
#include "../utility/utility.h"

int main(int argc, char* argv[]){
	//Struttura che contiene le informazioni del sistema
	struct sysinfo hostinfo;

	//Struttura che contiene nome ed indirizzo dell'host
	struct hostent *host;

	//Stringa che memorizza i dati dell'host
	char hostbuffer[256];

	//Variabili utilizzate per calcolare il tempo (Ora, data etc.)
	struct tm *hosttime;
	time_t rawtime;

	//Buffer dati da inviare al server
	char buffDati[200]; 

	//Stringa utilizzata per immagazzinare il nome dell'host
	char name_host[50]; 

	//Variabili utilizzate per il calcolo della percentuale della RAM
	long totram; //RAM totale
	double freeram; //RAM libera

	//Socket Descriptor
	int skagent;

	//Dichiaro un sockaddr del tipo del server
	struct sockaddr_in my_agent;

	//Inizializzo la struttura dell' indirizzo dell'Agent
	my_agent.sin_family = AF_INET;
	my_agent.sin_port = htons(atoi(argv[2]));
	my_agent.sin_addr.s_addr = inet_addr(argv[1]);


	//****************************************************************
	//*   Inizio invio costante di informazioni sul Socket skagent   *
	//****************************************************************
	while(1){	
		
		//***********************************************************
		//* Inizio apertura del Socket per la connessione al server *
		//***********************************************************

		skagent = socket(PF_INET, SOCK_STREAM, 0);
		if(connect(skagent, (struct sockaddr*)&my_agent, sizeof(my_agent)) < 0){
			printToMonitor("ERRORE: la connessione è fallita\n");
			exit(-1);	
		}
		
		//***********************************************************
		//* Inizio apertura del Socket per la connessione al server *
		//***********************************************************
		

		//Funzione utilizzata per reperire le informazioni dell'host
	   	if(gethostname(hostbuffer, sizeof(hostbuffer)) == -1){
			printToMonitor("ERRORE: non è stato possibile reperire le info dell'host\n");
			exit(-1);		
		}

		host = gethostbyname(hostbuffer);
		
		//Controllo se l'host ha un nome o meno
		if(strcmp(host->h_name, " ") != 0)
			strcpy(name_host, host->h_name);
		else
			strcpy(name_host, host->h_addr_list[0]); 
		
		//Calcolo della RAM libera
		totram = hostinfo.freeram + hostinfo.totalram + hostinfo.sharedram + hostinfo.bufferram;
		freeram = (float)(hostinfo.freeram)/totram;
	
		//Reperisco il TIMESTAMP 
		time(&rawtime);
		hosttime = localtime(&rawtime); 

		//Resetto il BuffDati, in modo tale da poterci scrivere altre informazioni.
		memset(buffDati, 0, sizeof(buffDati));

		//Creazione della stringa da inserire in buffDati
		sprintf(buffDati, "%s, %ld, %f%c, %hd,%02d:%02d:%02d %02d/%02d/%d", name_host, hostinfo.uptime, freeram*100, '%', hostinfo.procs, hosttime->tm_hour, hosttime->tm_min, hosttime->tm_sec, hosttime->tm_mday, (hosttime->tm_mon+1), (hosttime->tm_year+1900));

		//Invio delle informazioni dell'host al Server
		write(skagent, buffDati, strlen(buffDati));
		
		//Chiusura della connessione
		close(skagent);
		
		//Attesa di 3 secondi per l'invio di ulteriori informazioni
		sleep(3);	
	}
	//****************************************************************
	//*   Fine invio costante di informazioni sul Socket skagent     *
	//****************************************************************

}
