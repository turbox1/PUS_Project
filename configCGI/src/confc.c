#include <stdio.h>
#include <stdlib.h>
#include "cgi.h"

//Constants
#define FILE_BUF 8192
#define FILES_NUMB 8

//Variables
s_cgi *cgi;
FILE *pFile;
char filename[10];

//Functions
void init();
short saveConfig(unsigned short idx, char* data);
short loadConfig(unsigned short idx, char* data);
void loadAllConfig(char* data);
short clearConfig(unsigned short idx);

char* _action = NULL;
char* _id_conf = NULL;
char* _data = NULL;


//-------------------------------------------------------------------------------------------------
int main() {
	init();
	
	_action = cgiGetValue(cgi, "action");
	if(!_action) {
		const int buffer_size = FILE_BUF*FILES_NUMB;
		char buffer[buffer_size];
		
		loadAllConfig(buffer);
		printf(buffer);
		return 0;
	}
	
	_id_conf = cgiGetValue(cgi, "id");
	
	switch(atoi(_action)) {
	case 0: //Zapis konfiguracji
		_data = cgiGetValue(cgi, "data");
		saveConfig(atoi(_id_conf), _data);
		//printf("ZAPIS\n");
		break;
		
	case 1: //Usuwanie konfiguracji
		clearConfig(atoi(_id_conf));
		//printf("USUWANIE\n");
		break;
		
	default:
		break;
	}

	cgiFree(cgi);
	return 0;
}
//-------------------------------------------------------------------------------------------------


//Inicjuje dzialanie programu
void init() {
	pFile = NULL;
	
	//Inicjuje biblioteke do obsługi CGI
	cgi = cgiInit();
	cgiHeader();
}


//Zapisuje konfiguracje 'idx' z tablicy data
//Zwraca [ 1 gdy sukces ], [0 gdy porazka ]
short saveConfig(unsigned short idx, char* data) {
	sprintf(filename, "cfg/%d.conf", idx);

	pFile = fopen(filename, "w+");
	if(!pFile) {
		return 0;
	}

	fprintf(pFile, "%s", data);

	fclose(pFile);
	return 1;
}


//Wczytuje konfiguracje 'idx' do tablicy data
//Zwraca [ liczbe odczytanych znakow +1 gdy sukces], [ 0 gdy porazka ]
short loadConfig(unsigned short idx, char* data) {
	sprintf(filename, "cfg/%d.conf", idx);
	int ptr = 0;
	char tmpChar;

	pFile = fopen(filename, "r");
	if(!pFile) {
		return 0;
	}

	while((tmpChar=fgetc(pFile)) != EOF) {
		data[ptr++] = tmpChar;
	}
	data[ptr] = '\0';

	fclose(pFile);
	return ptr;
}


//Wczytuje wszystkie konfiguracje do tablicy data
void loadAllConfig(char* data) {
	unsigned short i;
	unsigned int ptr = 0;

	data[ptr++] = '[';
	for(i=0; i<FILES_NUMB; i++) {
		data[ptr++] = '{';
		ptr += loadConfig(i, &data[ptr]);
		data[ptr++] = '}';

		if(i!=FILES_NUMB-1) data[ptr++] = ',';
	}
	data[ptr++] = ']';
	data[ptr] = '\0';

	return;
}


//Czysci plik konfiguracyjny 'idx'
//Zwraca [1 gdy sukces], [0 gdy porazka]
short clearConfig(unsigned short idx) {
	sprintf(filename, "cfg/%d.conf", idx);

	pFile = fopen(filename, "w");
	if(!pFile) {
		return 0;
	}

	fclose(pFile);
	return 1;
}
