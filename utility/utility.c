#include "utility.h"


int sendString(int std, char* stringa, int strLength){
	//Bytes mancanti
	int missingBytes = strLength;
	int bytesWritten;

	//Ciclo scrivendo i bytes finchè non scrivo tutta la stringa
	do {
		bytesWritten = write(std, stringa, missingBytes);
		if (bytesWritten < 0){
			return -1;
		}	
		
		//Sposto il puntatore per continuare a scrivere da quel byte
		stringa += bytesWritten;
		//Decremento il numero di bytes mancanti da scrivere
		missingBytes -= bytesWritten;
	
	} while (missingBytes > 0);
	
	return 0;
	
}

void printToMonitor(char* message){
	sendString(STDOUT_FILENO, message, strlen(message));
}

void printErrorToMonitor(char* message){
	sendString(STDERR_FILENO, message, strlen(message));
}
