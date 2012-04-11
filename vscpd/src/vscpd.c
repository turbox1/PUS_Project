#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <time.h>
#include "vscp_frame.h"
#include "uart.h"

#define PORT 9999
#define DEV_NUMB 2
#define SENSOR_BUF_SIZE 8

//--- global ---
struct vscp_frame v_frame;
int temperature;
int humidity;

//--- thread_function ----
void* web_thread_fun(void* arg);
void* serial_thread_fun(void* arg);


int main() {
  int udp_fd;
  int serial_fd;
  int retval;
  struct sockaddr_in saddr;
  socklen_t saddr_len;
  pthread_t web_thread;
  pthread_t serial_thread;

  serial_fd = openPort("/dev/ttyS1", 115200);
  if(serial_fd == -1) {
    fprintf(stderr, "openPort()\n");
    exit(EXIT_FAILURE);
  }

  udp_fd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
  if(udp_fd == -1) {
    fprintf(stderr, "socket()\n");
    exit(EXIT_FAILURE);
  }

  saddr_len = sizeof(struct sockaddr_in);

  memset((void*)&saddr, 0, saddr_len);
  saddr.sin_family = AF_INET;
  saddr.sin_port = htons(PORT);
  saddr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);

  retval = bind(udp_fd, (struct sockaddr*)&saddr, saddr_len);
  if(retval != 0) {
    fprintf(stderr, "bind()\n");
    close(udp_fd);    
    exit(EXIT_FAILURE);
  }

  pthread_create(&web_thread, NULL, web_thread_fun, (void*)&udp_fd);
  pthread_create(&serial_thread, NULL, serial_thread_fun, (void*)&serial_fd);

  pthread_join(web_thread, NULL);
  pthread_join(serial_thread, NULL);
  return 0;
}


void* web_thread_fun(void* arg) {
  int udp_fd = *((int*)arg);
  printf("web_thread()\n");

  struct sockaddr_in saddr;
  socklen_t saddr_len;
  int retval, dev_numb;
  unsigned char i, x;
  time_t rawtime;
  struct vscp_frame* out_frame = (struct vscp_frame*)malloc(DEV_NUMB*sizeof(struct vscp_frame));

  dev_numb = DEV_NUMB;
  retval = 0;

  out_frame[0].v_class = 10;
  out_frame[0].v_type = 6;

  out_frame[1].v_class = 10;
  out_frame[1].v_type = 35;


  saddr_len = sizeof(saddr);
  
  while(1) {
    if(retval = recvfrom(udp_fd, &v_frame, sizeof(struct vscp_frame), 0, (struct sockaddr*)&saddr, &saddr_len)) {

      for(x=0; x<4; x++) {
	out_frame[0].v_data[4-x]=((uint8_t*)&temperature)[x];
	out_frame[1].v_data[4-x]=((uint8_t*)&humidity)[x];
      }
	
      // print_vscp_frame(&v_frame);
      time(&rawtime);
      printf("Temparture: %0.2lf\tHumidity: %0.2lf - %s", (float)(temperature/100.), (float)(humidity/100.), ctime(&rawtime));
      sendto(udp_fd, &dev_numb, sizeof(int), 0, (struct sockaddr*)&saddr, saddr_len);
      for(i=0; i<dev_numb; i++)
	sendto(udp_fd, &out_frame[i], sizeof(struct vscp_frame), 0, (struct sockaddr*)&saddr, saddr_len);
    }
  }

  pthread_exit(NULL);
}


void* serial_thread_fun(void* arg) {
  int serial_fd = *((int*)arg);
  printf("serial_thread()\n");
  
  unsigned char buf[SENSOR_BUF_SIZE];
  uint8_t len;
  uint8_t i;

  while(1) {
    if(write(serial_fd, "a", 1) == 0) {
      fprintf(stderr, "write()\n");
      break;
    }

    len = read(serial_fd, buf, SENSOR_BUF_SIZE);
    if(len != SENSOR_BUF_SIZE) {
      fprintf(stderr, "read()\n");
      break;
    }

    for(i=0; i<2; i++) {
      ((uint8_t*)&temperature)[1-i] = buf[i+1];
      ((uint8_t*)&humidity)[1-i] = buf[i+3];
    }      
    //   printf("Temperature: %d\tHumidity: %d\n", temperature, humidity);
    
    sleep(2);
  }
 
  pthread_exit(NULL);
}


