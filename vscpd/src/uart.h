/*
 * uart.h
 *
 *  Created on: 09-08-2011
 *      Author: turbox
 */

#ifndef UART_H_
#define UART_H_

#include <errno.h>
#include <fcntl.h>
#include <termios.h>

int openPort(char* port, long speed);
void set8N1(int uartPort);
void setSpeedPort(int uartPort, long speed);

#endif /* UART_H_ */
