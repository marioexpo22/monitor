#include <unistd.h>
#include <string.h>

struct DatiHost{
	char name_host[50];
	char date[20];
	char buffDati[200];
	int status; //0 disconnesso, 1 connesso 
};

int sendString(int, char*, int);
void printToMonitor(char*);
void printErrorToMonitor(char*);
