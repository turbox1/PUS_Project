#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>

#define PORT 9999;

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
  char buf[255];
  int retval = 0;

  memset((void*)buf, 0, sizeof(buf));
  saddr_len = sizeof(saddr);

  printf("Watek\n");

  while(1) {
    memset((void*)buf, 0, retval);
    if(retval = recvfrom(udp_fd, buf, sizeof(buf), 0, (struct sockaddr*)&saddr, &saddr_len)) {
      printf("%s\n", buf);
      sendto(udp_fd, "Agnieszka", 5, 0, (struct sockaddr*)&saddr, saddr_len);
    }
  }
}
