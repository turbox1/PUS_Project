#ifndef VSCP_CONFIG_H
#define VSCP_CONFIG_H

#include <stdio.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

//----------------------- define -------------------------
#define DLE 0x10
#define STX 0x02
#define ETX 0x03
#define BUF_SIZE 16 //wielkosc bufora dla portu szeregowego
#define PORT 9999 //port dla VSCPD
#define NODE_NUM 255 //maksymalna liczba wezlow
#define TIMEOUT_HEARTBEAT 3 //liczba nie powodzeń

//---------------------- typedef -------------------------
typedef unsigned char  uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int   uint32_t;


//------------------- file_descryptors --------------------
FILE* eeprom_fd; //wskaznik do pliku z eeprom
int serial_fd; //deskryptor portu szeregowego
int net_fd; //desktyptor socketu UDP


//------------------- global_variable ---------------------
struct sockaddr_in saddr;
socklen_t saddr_len;
uint8_t active_node; //zmienna przechowuje liczbe aktywnych wezlow
uint8_t node_list[NODE_NUM]; //zmienna przechowuje liczbe timeout'ow poszczegolnych wezlow

uint8_t node_unit[NODE_NUM];	//jednost temperatury dla poszczegolnych wezlow
short node_temperature[NODE_NUM]; //wartosc temperatury dla poszczegolnych wezlow
short node_humidity[NODE_NUM]; //wartosc wilgotnosci dla poszczegolnych wezlow
uint8_t node_light[NODE_NUM]; //stany przyciskow poszczegolnych wezlow

uint8_t len;
uint8_t buf[BUF_SIZE];
pthread_mutex_t serial_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t log_mutex = PTHREAD_MUTEX_INITIALIZER;

//mapa danych w pliku eeprom
//-------------------- eeprom_map -------------------------
#define EEPROM_CRC              0x00
#define EEPROM_NICKNAME         0x01
#define EEPROM_MAJOR_VERSION    0x02
#define EEPROM_MINOR_VERSION    0x03
#define EEPROM_SUBMINOR_VERSION 0x04
#define EEPROM_CTRL_BYTE        0x05
#define EEPROM_ZONE             0x06
#define EEPROM_SUBZONE          0x07
#define EEPROM_GUID             0x100
#define EEPROM_USER_ID			0x0D
#define EEPROM_MANUFACTURER		0x12
#define EEPROM_APP_REGISTER		0x1A

#endif
