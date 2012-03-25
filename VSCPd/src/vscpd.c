#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <time.h>
#include "../../VSCP/src/vscp_fun.h"

#define PORT 9999;
#define DEV_NUMB 2;

//--- global ---
struct vscp_frame v_frame;

//--- thread_function ----
void* web_thread_fun(void* arg);


int main() {
  int udp_fd;
  int retval;
  struct sockaddr_in saddr;
  socklen_t saddr_len;
  pthread_t web_thread;

  udp_fd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
  if(udp_fd == -1) {
    fprintf(stderr, "socket()\n");
    exit(EXIT_FAILURE);
  }

  saddr_len = sizeof(struct sockaddr_in);

  memset((void*)&saddr, 0, saddr_len);
  saddr.sin_family = AF_INET;
  saddr.sin_port = htons(9999);
  saddr.sin_addr.s_addr = htonl(INADDR_ANY);

  retval = bind(udp_fd, (struct sockaddr*)&saddr, saddr_len);
  if(retval != 0) {
    fprintf(stderr, "bind()\n");
    close(udp_fd);    
    exit(EXIT_FAILURE);
  }

  pthread_create(&web_thread, NULL, web_thread_fun, (void*)&udp_fd);
  pthread_join(web_thread, NULL);
  return 0;
}


void* web_thread_fun(void* arg) {
  int udp_fd = *((int*)arg);
  struct sockaddr_in saddr;
  socklen_t saddr_len;
  int retval = 0;
  int dev_numb = DEV_NUMB;

  struct vscp_frame* out_frame = (struct vscp_frame*)malloc(dev_numb*sizeof(struct vscp_frame));
  int i;

  time_t rawtime;
  
  srand(time(&rawtime));

  out_frame[0].v_class = 10;
  out_frame[0].v_type = 6;

  int temp = (rand()%2)*100 + (rand()%10)*10 + (rand()%10);
  for(i=0; i<4; i++) {
    out_frame[0].v_data[3-i]=((uint8_t*)&temp)[i];
  }

  out_frame[1].v_class = 10;
  out_frame[1].v_type = 35;

  saddr_len = sizeof(saddr);

  printf("web_thread()\n");

  while(1) {
    if(retval = recvfrom(udp_fd, &v_frame, sizeof(struct vscp_frame), 0, (struct sockaddr*)&saddr, &saddr_len)) {
      // print_vscp_frame(&v_frame);
      time(&rawtime);
      printf("Temp: %d -  %s", temp, ctime(&rawtime));
      sendto(udp_fd, &dev_numb, sizeof(int), 0, (struct sockaddr*)&saddr, saddr_len);
      for(i=0; i<dev_numb; i++)
	sendto(udp_fd, &out_frame[i], sizeof(struct vscp_frame), 0, (struct sockaddr*)&saddr, saddr_len);
    }
  }
}

