#ifndef VSCP_LOG_H
#define VSCP_LOG_H

#include <stdio.h>
#include <time.h>

FILE *log_fd;
char log_tmp[255];
time_t current_time;

void log_init();
void log_printf();
void log_fprintf(const char* msg);


#endif //VSCP_LOG_H
