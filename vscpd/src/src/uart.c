#include "uart.h"

//funkcja odpowiedzialna za otwarcie portu RS232 z okreslona predkoscia
//parametr "port" np. /dev/ttyS0
//dostepne predkosci:
//-115200 BAUD
//-57600 BAUD - ta predkosc wykorzystywana jest w projekcie 
//poniewaz z taka predkoscia wspolpracuje modul w VSCP WORKS 
int openPort(char* port, long speed) {
	int uartPort = open(port, O_RDWR | O_NOCTTY | O_NDELAY);
	if(uartPort == -1) {
		perror("openPort:");
	} else {
		fcntl(uartPort, F_SETFL, 0);
		setSpeedPort(uartPort, speed);
		set8N1(uartPort);
	}

	return uartPort;
}

//funkcja odpowiedzialna za ustawienie odpowiedniej predkosci dla portu
void setSpeedPort(int uartPort, long speed) {
	struct termios options;

	tcgetattr(uartPort, &options);

	switch(speed) {
		case 115200:
			cfsetispeed(&options, B115200);
			cfsetospeed(&options, B115200);
			break;
		case 57600:
			cfsetispeed(&options, B57600);
			cfsetospeed(&options, B57600);
			break;
		default:
			cfsetispeed(&options, B57600);
			cfsetospeed(&options, B57600);
			break;
	}

	options.c_cflag |= (CLOCAL | CREAD);

	tcsetattr(uartPort, TCSANOW, &options);
}


//ustawienie w tryb 8N1
//8 - bitow danych
//N - brak bitu parzystosci
//1 - bit stopu
void set8N1(int uartPort) {
	struct termios options;

	tcgetattr(uartPort, &options);

	options.c_cflag &= ~PARENB;
	options.c_cflag &= ~CSTOPB;
	options.c_cflag &= ~CSIZE;
	options.c_cflag |= CS8;

	options.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG); //=0
	options.c_oflag &= ~OPOST; //=0
	options.c_cc[VMIN]=1;
	options.c_cc[VTIME]=0;

	tcsetattr(uartPort, TCSANOW, &options);
}
