#include <stdio.h>
#include <stdlib.h>
#include "vscpc_cfg.h"


//void print_all_frame(const int dev_numb, struct vscp_frame* vf);
//char* give_params(struct vscp_frame* vf);


int main() {
  udp_fd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
  if(udp_fd == -1) {
    fprintf(stderr, "socket()");
    exit(EXIT_FAILURE);
  }
  
  active_node = 0;
  daddr_len = sizeof(struct sockaddr_in);
  memset((void*)&daddr, 0, daddr_len);
  daddr.sin_family = AF_INET;
  daddr.sin_port = htons(9999);
  daddr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);

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
  struct _msg vscp_msg;
  size_t vscp_msg_len = sizeof(struct _msg);
  memset((void*)&vscp_msg, 0, vscp_msg_len);

  char* _class = NULL;
  char* _type = NULL;
  char* _id = NULL;
  char* _val = NULL;
  char* _repeat = NULL;
  short _value = 0;
  
  cgi = cgiInit();
  cgiHeader();
  _class = cgiGetValue(cgi, "class");
  _type = cgiGetValue(cgi, "type");
  
  vscp_msg.class = atoi(_class);
  vscp_msg.type = atoi(_type);

  if(vscp_msg.class == 20 && vscp_msg.type == 1) {
    _id = cgiGetValue(cgi, "id");
    _val = cgiGetValue(cgi, "value");
    _repeat = cgiGetValue(cgi, "repeat");
    vscp_msg.oaddr = atoi(_id);
    vscp_msg.data[0] = atoi(_val) | (atoi(_repeat) << 3);
    rv = sendto(udp_fd, &vscp_msg, vscp_msg_len, 0, (struct sockaddr*)&daddr, daddr_len);
    printf("\n");
  } else {
	rv = sendto(udp_fd, &vscp_msg, vscp_msg_len, 0, (struct sockaddr*)&daddr, daddr_len);
	rv = recvfrom(udp_fd, &active_node, 1, 0, (struct sockaddr*)&daddr, &daddr_len);
  
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
  
  close(udp_fd);
  cgiFree(cgi);
  return 0;
}


