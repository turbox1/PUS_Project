#ifndef VSCPC_CFG_H
#define VSCPC_CFG_H


//---------------------- include ----------------------
#include <string.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <cgi.h>


//---------------------- defines ----------------------
#define SUCCESS "\"success\""
#define CLASS "\"class\""
#define TYPE "\"type\""
#define ID "\"id\""
#define PARAMS "\"params\""
#define VAL "\"value\""
#define UNIT "\"unit\""

//--------------------- type_def ----------------------
typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int uint32_t;

struct _msg {
	/*!
		Input message flags\n
		==================\n
		Bit 7 - Set if message valid\n
		Bit 6 - Reserved\n
		Bit 5 - Hardcoded (will never be set)\n
		Bit 3 - Number of data bytes MSB\n
		Bit 2 - Number of data bytes \n
		Bit 1 - Number of data bytes\n
		Bit 0 - Number of data bytes LSB\n
	*/
	uint8_t flags;		///< Input message flags

	uint8_t priority;	///< Priority for the message 0-7	
	uint16_t class;		///< VSCP class
	uint8_t type;		///< VSCP type
	uint8_t oaddr;		///< Packet originating address
	uint8_t data[8];	///< data bytes 	
};


//------------------ global variable ------------------
int udp_fd;
struct sockaddr_in daddr;
socklen_t daddr_len;
uint8_t active_node;
int rv;
s_cgi *cgi;

#endif
