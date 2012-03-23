#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <cgi.h>
#include <time.h>

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

  char buf[255];
  
  cgi_init();
  cgi_process_form();
  cgi_init_headers();

  strcpy(buf, cgi_param("cls"));


  sendto(udp_fd, buf, sizeof(buf), 0, (struct sockaddr*)&daddr, daddr_len);
  recvfrom(udp_fd, buf, sizeof(buf), 0, (struct sockaddr*)&daddr, &daddr_len);
  printf("%s\n", buf);

  close(udp_fd);
  cgi_end();
  return 0;
}
