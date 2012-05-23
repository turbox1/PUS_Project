#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include "src/uart.h"
#include "src/vscp_config.h"
#include "src/vscp_firmware.h"
#include "src/vscp_class.h"
#include "src/vscp_type.h"


//---------------------- extern_variable ------------------
extern uint8_t vscp_second;

//---------------------- global_variable ------------------
pthread_t web_thread;
pthread_t serial_thread;
pthread_t one_second;
pthread_mutex_t serial_mutex;


//------------------------ functions ----------------------
void init_daemon();
void init_eeprom();


//----------------------- threads_fun ---------------------
void* web_thread_fun(void* arg);
void* serial_thread_fun(void* arg);
void* one_second_thread_fun(void* arg);


//--------------------------- main ------------------------
int main(int argc, char* argv[]) {
  init_daemon(); //rozpoczynanie pracy daemona
  
  eeprom_fd = fopen("/www/eeprom.cfg", "r+b");  
  if(eeprom_fd == NULL)	eeprom_fd = fopen("/www/eeprom.cfg", "w+b");  
  
  if(!vscp_check_pstorage()) init_eeprom();
  
  vscp_init();
  printf("NICKNAME = 0x%x\n", vscp_nickname);
  
  while(1) {
	  switch(vscp_node_state) {
		  case VSCP_STATE_STARTUP:
				if(VSCP_ADDRESS_FREE == vscp_nickname) {
					vscp_node_state = VSCP_STATE_INIT;
				} else {
					vscp_node_state = VSCP_STATE_ACTIVE;
					vscp_goActiveState();
				}
				break;
				
	      case VSCP_STATE_INIT:
				vscp_handleProbeState();
				break;
				
		  case VSCP_STATE_PREACTIVE:
				vscp_goActiveState();
				break;
				
		  case VSCP_STATE_ACTIVE:
				if(vscp_imsg.flags & VSCP_VALID_MSG) {
					vscp_handleProtocolEvent();
					vscp_imsg.flags = 0;
				}
				break;
				
		  case VSCP_STATE_ERROR:
				printf("ERROR :: Exit(-1);\n");
				exit(-1);
				break;
	  }
  }
  
  pthread_join(serial_thread, NULL);
  pthread_join(web_thread, NULL);
  pthread_join(one_second, NULL);

  fclose(eeprom_fd);
  close(net_fd);
  close(serial_fd);
 
  return 0;
}



//---------------------------------------------------------
void init_daemon() {
  printf("init_daemon\n");

  pthread_create(&web_thread, NULL, web_thread_fun, NULL);
  pthread_create(&serial_thread, NULL, serial_thread_fun, NULL);
  pthread_create(&one_second, NULL, one_second_thread_fun, NULL);

  serial_fd = -1;
  serial_fd = openPort("/dev/ttyS1", 57600);
  
  net_fd = 0;
  net_fd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
  if(net_fd == -1) {
	  fprintf(stderr, "socket()\n");
	  exit(EXIT_FAILURE);
  }
  
  saddr_len = sizeof(struct sockaddr_in);
  memset((void*)&saddr, 0, saddr_len);
  saddr.sin_family = AF_INET;
  saddr.sin_port = htons(PORT);
  saddr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);

  if(bind(net_fd, (struct sockaddr*)&saddr, saddr_len) != 0) {
	  fprintf(stderr, "bind()\n");
	  close(net_fd);
	  exit(EXIT_FAILURE);
  }

  eeprom_fd = NULL;
  
  for(active_node=0; active_node<NODE_NUM; active_node++) {
	  node_light[active_node] = 0;
	  node_list[active_node] = 0;
  }
  active_node = 0;

  
}


//------------------------ = ---------------------------------
void* web_thread_fun(void* arg) {
  printf("web_thread_fun()\n");
  uint8_t x, i;
  int rv;
  
  struct _imsg web_msg;
  size_t web_msg_len = sizeof(struct _imsg);
  memset((void*)&web_msg, 0, web_msg_len);
   
  while(1) {
		rv = recvfrom(net_fd, &web_msg, web_msg_len, 0, (struct sockaddr*)&saddr, &saddr_len);
    	if(web_msg.class == VSCP_CLASS1_DATA && web_msg.type == VSCP_TYPE_UNDEFINED) {
    		rv = sendto(net_fd, &active_node, 1, 0,(struct sockaddr*)&saddr, saddr_len);
    		i=0;
	  
			for(x=0; x<active_node; x++) {
      				for(i; i<NODE_NUM; i++) {
					if(node_list[i] != 0) {
						web_msg.oaddr = i;
						web_msg.class = VSCP_CLASS1_MEASUREMENT;
				 
						//temperature
						web_msg.type = VSCP_TYPE_MEASUREMENT_TEMPERATURE;
						web_msg.data[0] = node_unit[i];
						
						memcpy((void*)&web_msg.data[1], (void*)&node_temperature[i], sizeof(short));
						rv = sendto(net_fd, &web_msg, web_msg_len, 0, (struct sockaddr*)&saddr, saddr_len);
			//			printf("Send rv = %d\n", rv);
			   
						//humidity
						web_msg.type = VSCP_TYPE_MEASUREMENT_HUMIDITY;
						memcpy((void*)web_msg.data, (void*)&node_humidity[i], sizeof(short));
						rv = sendto(net_fd, &web_msg, web_msg_len, 0, (struct sockaddr*)&saddr, saddr_len);
					
						//button - response
						web_msg.class = VSCP_CLASS1_INFORMATION;
						web_msg.type = node_light[web_msg.oaddr]?VSCP_TYPE_INFORMATION_ON:VSCP_TYPE_INFORMATION_OFF;
						//if(node_light[web_msg.oaddr] != 1) web_msg.type = VSCP_TYPE_INFORMATION_ON;
						//else web_msg.type = VSCP_TYPE_INFORMATION_OFF;
						web_msg.data[0] = 0x01;
						web_msg.data[1] = vscp_getZone();
						web_msg.data[2] = vscp_getSubzone();	
						rv = sendto(net_fd, &web_msg, web_msg_len, 0, (struct sockaddr*)&saddr, saddr_len);
					}	
				}
			}
			//printf("web_thread_fun() :: SEND\n");
		} 
		if(web_msg.class == VSCP_CLASS1_INFORMATION && web_msg.type == VSCP_TYPE_INFORMATION_BUTTON) {
			//node_light[0x04] = (web_msg.data[0] & 0x07);
			//printf("val :%d\n", ((web_msg.data[0] & 0xF8) >> 3));
			web_msg.data[0] = 0x02 | (web_msg.data[0] & 0xF8);
			web_msg.data[1] = vscp_getZone();
			web_msg.data[2] = vscp_getSubzone();
			web_msg.data[3] = 0x00;
			web_msg.data[4] = 0x01;
			web_msg.data[5] = 0x00;
			web_msg.data[6] = 0x00;
			if(sendVSCPFrame(web_msg.class, web_msg.type, web_msg.oaddr, web_msg.priority, 0x07, web_msg.data))
		
			printf("WEB -> Button NODE :: [ %x ]\t [ REPEAT = %d ]\n", web_msg.oaddr, ((web_msg.data[0]&0xF8) >> 3));
		}	
  }
  pthread_exit(NULL);
}


//---------------------------------------------------------
void* serial_thread_fun(void* arg) {
  printf("serial_thread_fun()\n");
  len = 0;
  memset(buf, 0, BUF_SIZE);
  
  do {
    len += read(serial_fd, &buf[len], 1);
    if(len > 2 && buf[len-2] == DLE && buf[len-1] == ETX) {
	  while(pthread_mutex_trylock(&serial_mutex));
      vscp_getEvent();
      //printf("Len = %d\n", len);
      pthread_mutex_unlock(&serial_mutex);
      memset(buf, 0, BUF_SIZE);
      len = 0;
    } 
    if(len>15) len = 0;   
  } while(len != -1);
  
  close(serial_fd);
  pthread_exit(NULL);
}


void* one_second_thread_fun(void* arg) {
  volatile uint8_t x = 0;
  uint8_t i = 0;
  while(1) {
	++vscp_timer;
	if(vscp_timer == 1000) {
		vscp_doOneSecondWork();
		vscp_timer = 0;
		
		if(active_node) ++x;
		if(x==30) {
			for(i=0; i<NODE_NUM; i++) {
				if(node_list[i] != 0) {
					node_list[i]--;
					if(node_list[i] == 0) {
						--active_node;
						printf("Disconnected NODE :: [ %x ]\n", i);
					}
				}
				
			}
			x = 0;
		}
	}
    usleep(900);
  }
}


void init_eeprom() {
	uint8_t e_init[128];
	memset(e_init, 0, sizeof(e_init));
	printf("init_eeprom()\n");
	
	e_init[EEPROM_CRC] = 0x40;
	e_init[EEPROM_NICKNAME] = 0x02;
	e_init[EEPROM_MAJOR_VERSION] = 0;
	e_init[EEPROM_MINOR_VERSION] = 0;
	e_init[EEPROM_SUBMINOR_VERSION] = 1;
	e_init[EEPROM_CTRL_BYTE] = 0;
	e_init[EEPROM_ZONE] = 0;
	e_init[EEPROM_SUBZONE] = 0;
	
	e_init[EEPROM_APP_REGISTER+1] = 0x11;
	e_init[EEPROM_APP_REGISTER+8] = 0x23;
	
	fseek(eeprom_fd, EEPROM_CRC, SEEK_SET);
	fwrite(e_init, 1, sizeof(e_init), eeprom_fd);
	fflush(eeprom_fd);
}
