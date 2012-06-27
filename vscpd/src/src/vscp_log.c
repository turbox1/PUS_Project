#include "vscp_log.h"

void log_init() {
  log_fd = 0;
  log_fd = fopen("/tmp/vscp.log", "a+");
  if(!log_fd) {
    fprintf(stderr, "Error :: vscp.log\n");
    return;
  }
  printf("Opened :: [ vscp.log ]\n");
}

void log_printf() {
  time(&current_time);
  fprintf(log_fd, "%s%s", ctime(&current_time), log_tmp);
  printf("%s", log_tmp);
  fflush(log_fd);
}

void log_fprintf(const char* msg) {
  time(&current_time);
  fprintf(log_fd, "%s%s", ctime(&current_time), msg);
  printf("%s", msg);
  fflush(log_fd);
}
