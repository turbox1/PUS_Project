#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <cgi.h>
#include "vscp_frame.h"

#define CLASS "\"class\""
#define TYPE "\"type\""
#define ID "\"id\""
#define PARAMS "\"params\""
#define VAL "\"value\""
#define UNIT "\"unit\""


//--- function ---
void print_all_frame(const int dev_numb, struct vscp_frame* vf);
char* give_params(struct vscp_frame* vf);

//--- global_variable ---
s_cgi *cgi;


int main() {
  int udp_fd;
  struct sockaddr_in daddr;
  socklen_t daddr_len;

  udp_fd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
  if(udp_fd == -1) {
    fprintf(stderr, "socket()");
    exit(EXIT_FAILURE);
  }
  
  daddr_len = sizeof(struct sockaddr_in);
  memset((void*)&daddr, 0, daddr_len);
  daddr.sin_family = AF_INET;
  daddr.sin_port = htons(9999);
  daddr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);

  char* _class = NULL;
  char* _type = NULL;
  int  dev_numb = -1;
  unsigned char  i;
  struct vscp_frame* in_frame = NULL;
  
  cgi = cgiInit();
  cgiHeader();
  
  _class = cgiGetValue(cgi, "class");
  _type = cgiGetValue(cgi, "type");


  struct vscp_frame v_frame;
  memset((void*)&v_frame, 0, sizeof(struct vscp_frame));
  v_frame.v_class = atoi(_class);
  v_frame.v_type = atoi(_type);
  

  sendto(udp_fd, &v_frame, sizeof(struct vscp_frame), 0, (struct sockaddr*)&daddr, daddr_len);
  recvfrom(udp_fd, &dev_numb, sizeof(int), 0, (struct sockaddr*)&daddr, &daddr_len);
  in_frame = (struct vscp_frame*)malloc(dev_numb*sizeof(struct vscp_frame));
  for(i=0; i<dev_numb; i++) {
    recvfrom(udp_fd, &in_frame[i], sizeof(struct vscp_frame), 0, (struct sockaddr*)&daddr, &daddr_len);
  }
				       
  print_all_frame(dev_numb, in_frame);


  close(udp_fd);
  cgiFree(cgi);
  return 0;
}


void print_all_frame(const int dev_numb, struct vscp_frame* vf) {
  printf("[");
  int i;
  for(i=0; i<dev_numb; i++) {
    char* ttmp = give_params(&vf[i]);
    printf("{%s: \"%d\", %s: \"%d\", %s: \"%d\", %s: {%s}}", CLASS, vf[i].v_class, TYPE, vf[i].v_type, ID, vf[i].v_data[7], PARAMS, ttmp);
    if(i<dev_numb-1) printf(", ");
    free(ttmp);
  }
  printf("]");
}


char* give_params(struct vscp_frame* vf) {
  char *tmp = (char*)malloc(128*sizeof(unsigned char));
  if(vf->v_class == 10) {
    int i;
    int val;
    for(i=0; i<4; i++) ((uint8_t*)&val)[i] = vf->v_data[4-i];

    switch(vf->v_type) {
    case 6:
      sprintf(tmp, "%s: \"%lf\", %s: \"%d\"", VAL, (float)(val/100.), UNIT, 1);
      break;
    
    case 35:
      sprintf(tmp, "%s: \"%lf\"", VAL, (float)(val/100.));
      break;
    }
  }
  return tmp;
}