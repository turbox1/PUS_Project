#include <stdio.h>
#include <stdlib.h>
#include "vscpc_cfg.h"

//VSCPD - Daemon VSCP

int main() {
  //tworze socket UDP do komunikacji z VSCPD
  udp_fd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
  if(udp_fd == -1) {
    fprintf(stderr, "socket()");
    exit(EXIT_FAILURE);
  }

//liczba aktywnych wezlow odbierana po wyslaniu zapytania class=15, type=0 od VSCPD
  active_node = 0;
  
//wypelniam strukture dla socketu
//usluga pracuje na porcie 9999
//serwer nasluchuje na adresie 127.0.0.1:9999
  daddr_len = sizeof(struct sockaddr_in);
  memset((void*)&daddr, 0, daddr_len);
  daddr.sin_family = AF_INET;
  daddr.sin_port = htons(9999);
  daddr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);

//przyklad struktury jaka przekazuje do przegladarki

//class 10 - Measurement
//type 6 - Temperature
//type 35 - Humidity

//class 20 - Information
//type 3 - ON
//type 4 - OFF

/*
{
    "success":true,
    "data":[
        {"class":"10", "type":"6", "id":"22", "params": {"unit":"0", "value":"12"}},
        {"class":"10", "type":"35", "id":"3", "params": {"value":"23"}},
        {"class":"10", "type":"6",  "id":"2", "params": {"value":"11"}},
        {"class":"10", "type":"35", "id":"4", "params": {"value":"41"}},
        {"class":"20", "type":"3",  "id":"43", "params": {}}
    ]
}
*/

//struktura wiadomosci VSCP
//definicja znaduje sie w "vscp_cfg.h"
  struct _msg vscp_msg;
  size_t vscp_msg_len = sizeof(struct _msg);
  memset((void*)&vscp_msg, 0, vscp_msg_len);

//zmienne w ktorych beda przechowywane zmienne z przegladarki
//zmienne wypelniane przez biblioteke CGI
  char* _class = NULL;
  char* _type = NULL;
  char* _id = NULL;
  char* _val = NULL;
  char* _repeat = NULL;
  short _value = 0;
  
//inicjuje biblioteke CGI
  cgi = cgiInit();
  
//funkcja biblioteczna - wysyla header do przegladarki
  cgiHeader();
  
//pobieram wartosci class i type z przegladarki po wyslaniu zapytania
//korzystam z funkcji bibliotecznych cgiGetValue;
  _class = cgiGetValue(cgi, "class");
  _type = cgiGetValue(cgi, "type");
 
//wypełniam wiadomosc klasa i typem
  vscp_msg.class = atoi(_class);
  vscp_msg.type = atoi(_type);

//sprawdzam jakie jest zapytanie
//class 20 - Information
//type 1 - button
//wysylam rozne zapytania w zalnosci od tego czy zmienia sie stan przycisku czy nie
//class 15 - Data
//type 0 - zapytanie o wszystkie dane
  if(vscp_msg.class == 20 && vscp_msg.type == 1) {

//pobieram pozostale wartosci dla przycisku
//repeat - moze byc interpretowany jako jasnosc
//id - nr_wezla
//value :: 0 - wylaczony / 1 - wlaczony
    _id = cgiGetValue(cgi, "id");
    _val = cgiGetValue(cgi, "value");
    _repeat = cgiGetValue(cgi, "repeat");
    vscp_msg.oaddr = atoi(_id); 
    vscp_msg.data[0] = atoi(_val) | (atoi(_repeat) << 3);
    rv = sendto(udp_fd, &vscp_msg, vscp_msg_len, 0, (struct sockaddr*)&daddr, daddr_len);
    printf("\n");
  } else {
	//wysylam ramke do VSCPD
	rv = sendto(udp_fd, &vscp_msg, vscp_msg_len, 0, (struct sockaddr*)&daddr, daddr_len);
	//odbieram liczbe hostow od VSCPD
	rv = recvfrom(udp_fd, &active_node, 1, 0, (struct sockaddr*)&daddr, &daddr_len);
  
 //odbieram odpowiednie informacji i przygotowuje odpowiednie obiekty
 //wysylam odpowiednio przygotowane obiekty do przegladarki
 //obiekt zawiera odpowiednie dane, ktore nastepnie zinterpretuje 
 //JavaScript na stronie!
 //korzystam z JSON - JavaScrip Object Notation
  printf("{\"success\":true,\n"
		 "\"data\": [\n");
  uint8_t x;
  for(x=0; x<active_node; x++) {
	  //temperature
	  recvfrom(udp_fd, &vscp_msg, vscp_msg_len, 0, (struct sockaddr*)&daddr, &daddr_len);
	  memcpy(&_value, &vscp_msg.data[1], sizeof(short));
	  printf("{%s:\"%d\", %s:\"%d\", %s:\"%d\", %s: {%s:\"%d\", %s:\"%lf\"}},\n", CLASS, vscp_msg.class, TYPE, vscp_msg.type, ID, vscp_msg.oaddr, PARAMS, UNIT, vscp_msg.data[0]&0x0F, VAL, (float)(_value/100.));
	  
	  //humidity
	  recvfrom(udp_fd, &vscp_msg, vscp_msg_len, 0, (struct sockaddr*)&daddr, &daddr_len);
	  memcpy(&_value, &vscp_msg.data[0], sizeof(short));
	  printf("{%s:\"%d\", %s:\"%d\", %s:\"%d\", %s: {%s:\"%lf\"}},\n", CLASS, vscp_msg.class, TYPE, vscp_msg.type, ID, vscp_msg.oaddr, PARAMS, VAL, (float)(_value/100.));
	  
	  //button
	  recvfrom(udp_fd, &vscp_msg, vscp_msg_len, 0, (struct sockaddr*)&daddr, &daddr_len);
	  printf("{%s:\"%d\", %s:\"%d\", %s:\"%d\"}", CLASS, vscp_msg.class, TYPE, vscp_msg.type, ID, vscp_msg.oaddr);
	  
	  if(x!=active_node-1) printf(",");
	  printf("\n");
  }
  printf(	"]}");
  }
  
  //zamkyam socket
  close(udp_fd);
  //czyscze pamiec po cgi
  cgiFree(cgi);
  return 0;
}

//dokladnie wszystkie ramki zostana omowione w dokumentacji
