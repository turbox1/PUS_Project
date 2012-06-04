///////////////////////////////////////////////////////////////////////////////
// File: vscp.c
//

/* ******************************************************************************
 * 	VSCP (Very Simple Control Protocol) 
 * 	http://www.vscp.org
 *
 * 	akhe@eurosource.se
 *
 *  Copyright (C) 1995-2011 Ake Hedman, 
 *  eurosource, <akhe@eurosource.se> 
 *
 * This software is provided 'as-is', without any express or implied
 * warranty.  In no event will the authors be held liable for any damages
 * arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software. If you use this software
 *    in a product, an acknowledgment in the product documentation would be
 *    appreciated but is not required.
 * 2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 * 3. This notice may not be removed or altered from any source distribution.
 * 
 *	This file is part of VSCP - Very Simple Control Protocol 	
 *	http://www.vscp.org
 *
 * ******************************************************************************
*/


//
// $RCSfile: vscp.c,v $
// $Revision: 1.6 $
//

#include <string.h>
#include <stdlib.h>
#include "vscp_firmware.h"
#include "vscp_class.h"	
#include "vscp_type.h"	
#include "vscp_config.h"


#ifndef FALSE
#define FALSE           0
#endif

#ifndef TRUE
#define TRUE            !FALSE
#endif

#ifndef ON
#define ON              !FALSE
#endif

#ifndef OFF
#define OFF             FALSE
#endif 


// Globals

// VSCP Data
uint8_t vscp_nickname;              ///< Node nickname

uint8_t vscp_errorcnt;              // VSCP/CAN errors
uint8_t vscp_alarmstatus;           // VSCP Alarm Status

uint8_t vscp_node_state;            // State machine state
uint8_t vscp_node_substate;         // State machine substate

uint8_t vscp_probe_cnt;             // Number of timout probes

// Incoming message
struct _imsg vscp_imsg;

// Outgoing message
struct _omsg vscp_omsg;

uint8_t vscp_probe_address;         // Address used during initialization
uint8_t vscp_initledfunc;           // Init LED functionality

volatile uint16_t vscp_timer;       // 1 ms timer counter 
                                    //	Shold be externally updated.

volatile uint8_t vscp_initbtncnt;   // init button counter
                                    //  increase this value by one each millisecond
                                    //  the initbutton is pressed. Set to sero when button
                                    // is released.

volatile uint8_t vscp_statuscnt;    // status LED counter
                                    //	increase bye one every millisecond 

// page selector
uint16_t vscp_page_select;

// The GUID reset is used when the VSCP_TYPE_PROTOCOL_RESET_DEVICE
// is received. Bit 4,5,6,7 is set for each received frame with 
// GUID data. Bit 4 is for index = 0, bit 5 is for index = 1 etc.
// This means that a bit is set if a frame with correct GUID is
// received.  
uint8_t vscp_guid_reset;

// Timekeeping
uint8_t vscp_second;
uint8_t vscp_minute;
uint8_t vscp_hour;


///////////////////////////////////////////////////////////////////////////////
// vscp_init
//

void vscp_init( void )
{
	vscp_initledfunc = VSCP_LED_BLINK1;

	// read the nickname id
	vscp_nickname = vscp_readNicknamePermanent();		

	//	if zero set to uninitialized
	if ( !vscp_nickname ) vscp_nickname = VSCP_ADDRESS_FREE;

	// Init incoming message
	vscp_imsg.flags = 0;	
	vscp_imsg.priority = 0;
	vscp_imsg.class = 0;
	vscp_imsg.type = 0;

	// Init outgoing message
	vscp_omsg.flags = 0;
	vscp_omsg.priority = 0;	
	vscp_omsg.class = 0;
	vscp_omsg.type = 0;

  	vscp_errorcnt = 0;					// No errors yet
  	vscp_alarmstatus = 0;				// No alarmstatus

	vscp_probe_address = 0;

	// Initial state
	vscp_node_state = VSCP_STATE_STARTUP;
	vscp_node_substate = VSCP_SUBSTATE_NONE;

	vscp_probe_cnt = 0;
	vscp_page_select = 0;
	
	// Initialize time keeping
	vscp_timer = 0;
	vscp_second = 0;
	vscp_minute = 0;
	vscp_hour = 0;
}

///////////////////////////////////////////////////////////////////////////////
// vscp_check_pstorage
//
// Check control position integrity and restore EEPROM
// if invalid.
//

int8_t vscp_check_pstorage( void )
{
	// controlbyte == 01xxxxxx means initialized
	// everything else is uninitialized
	if ( ( vscp_getSegmentCRC() & 0xc0 ) == 0x40 ) {
		return TRUE;
	}

	// No nickname yet.
	vscp_writeNicknamePermanent( 0xff );

	// No segment CRC yet.
	vscp_setSegmentCRC( 0x00 );

	// Initial startup
	// write allowed
	vscp_setControlByte( 0x90 );

	return FALSE;
}

///////////////////////////////////////////////////////////////////////////////
// vscp_error
//

void vscp_error( void )
{
	vscp_initledfunc = VSCP_LED_OFF;
}

///////////////////////////////////////////////////////////////////////////////
// vscp_handleProbe
//

void vscp_handleProbeState( void )
{
	switch ( vscp_node_substate ) {
	
		case VSCP_SUBSTATE_NONE:

			if ( VSCP_ADDRESS_FREE != vscp_probe_address ) {
				
				vscp_omsg.flags = VSCP_VALID_MSG + 1 ;	// one databyte 
				vscp_omsg.priority = VSCP_PRIORITY_HIGH;	
				vscp_omsg.class = VSCP_CLASS1_PROTOCOL;
				vscp_omsg.type = VSCP_TYPE_PROTOCOL_NEW_NODE_ONLINE;
				vscp_omsg.data[ 0 ] = vscp_probe_address;			
				
				// send the probe
				vscp_sendEvent();

				vscp_node_substate = VSCP_SUBSTATE_INIT_PROBE_SENT;
				vscp_timer = 0;	
				
			}
			else {

				// No free address -> error
				vscp_node_state = VSCP_STATE_ERROR;
				
				// Tell system we are giving up
				vscp_omsg.flags = VSCP_VALID_MSG + 1 ;	// one databyte 
				vscp_omsg.data[ 0 ] = 0xff;				// we are unassigned
				vscp_omsg.priority = VSCP_PRIORITY_LOW;
				vscp_omsg.class = VSCP_CLASS1_PROTOCOL;
				vscp_omsg.type = VSCP_TYPE_PROTOCOL_PROBE_ACK;			
				
				// send the error event
				vscp_sendEvent();

			}
			break;	

		case VSCP_SUBSTATE_INIT_PROBE_SENT:
			
			if ( vscp_imsg.flags & VSCP_VALID_MSG ) {	// incoming message?
				
				// Yes, incoming message
				if ( ( VSCP_CLASS1_PROTOCOL == vscp_imsg.class ) && 
					( VSCP_TYPE_PROTOCOL_PROBE_ACK == vscp_imsg.type ) ) {

					// Yes it was an ack from the segment master or a node
					if ( 0 == vscp_probe_address ) {

						// Master controller answered
						// wait for address
						vscp_node_state = VSCP_STATE_PREACTIVE;
						vscp_timer = 0; // reset timer
						
					}
					else {

						// node answered, try next address
						vscp_probe_address++;
						vscp_node_substate = VSCP_SUBSTATE_NONE;
						vscp_probe_cnt = 0;
						
					}	
				}
			}
			else {

				if ( vscp_timer > VSCP_PROBE_TIMEOUT ) {	// Check for timeout
 
					vscp_probe_cnt++;	// Another timeout
					
					if ( vscp_probe_cnt > VSCP_PROBE_TIMEOUT_COUNT ) { 
					
						// Yes we have a timeout
						if ( 0 == vscp_probe_address ) {			// master controller probe?
	
							// No master controler on segment, try next node
							vscp_probe_address++;
							vscp_node_substate = VSCP_SUBSTATE_NONE;
							vscp_timer = 0;
						}
						else {
							 
							// We have found a free address - use it
							vscp_nickname = vscp_probe_address;
							vscp_node_state = VSCP_STATE_ACTIVE;
							vscp_node_substate = VSCP_SUBSTATE_NONE;
							vscp_writeNicknamePermanent( vscp_nickname );
							vscp_setSegmentCRC( 0x40 );			// segment code (non server segment )

							// Report success
							vscp_probe_cnt = 0;
							vscp_goActiveState();
						
						}
					}
					else {
						vscp_node_substate = VSCP_SUBSTATE_NONE;
					}
				} // Timeout

			}			
			break;

		case VSCP_SUBSTATE_INIT_PROBE_ACK:
			break;

		default:
			vscp_node_substate = VSCP_SUBSTATE_NONE;
			break;
	}

	vscp_imsg.flags = 0;	
	
}

///////////////////////////////////////////////////////////////////////////////
// vscp_handlePreActiveState
//

void vscp_handlePreActiveState( void )
{
	if ( vscp_imsg.flags & VSCP_VALID_MSG ) {	// incoming message?

		if ( ( VSCP_CLASS1_PROTOCOL == vscp_imsg.class )  && 
						(  VSCP_TYPE_PROTOCOL_SET_NICKNAME == vscp_imsg.type ) &&
						(  VSCP_ADDRESS_FREE == vscp_imsg.data[ 0 ] ) ) {
						
			// Assign nickname
			vscp_nickname = vscp_imsg.data[ 1 ];
			vscp_writeNicknamePermanent( vscp_nickname );	
			vscp_setSegmentCRC( 0x40 );
	
			// Go active state
			vscp_node_state = VSCP_STATE_ACTIVE;						
		}		
	}
	else {
		// Check for time out
		if ( vscp_timer > VSCP_PROBE_TIMEOUT ) {
			// Yes, we have a timeout
			vscp_nickname = VSCP_ADDRESS_FREE;
			vscp_writeNicknamePermanent( VSCP_ADDRESS_FREE );
			vscp_init();	
		}				
	}
}

///////////////////////////////////////////////////////////////////////////////
// vscp_goActiveState
//

void vscp_goActiveState( void )
{
	vscp_omsg.flags = VSCP_VALID_MSG + 1 ;	// one databyte 
	vscp_omsg.priority = VSCP_PRIORITY_HIGH;	
	vscp_omsg.class = VSCP_CLASS1_PROTOCOL;
	vscp_omsg.type = VSCP_TYPE_PROTOCOL_NEW_NODE_ONLINE;
	vscp_omsg.data[ 0 ] = vscp_nickname;			
				
	// send the message
	vscp_sendEvent();

	vscp_initledfunc = VSCP_LED_ON;
}



///////////////////////////////////////////////////////////////////////////////
// vscp_sendHeartbeat
//

void vscp_sendHeartBeat( uint8_t zone, uint8_t subzone )
{
	vscp_omsg.flags = VSCP_VALID_MSG + 3 ;	// three databyte 
	vscp_omsg.priority = VSCP_PRIORITY_LOW;	
	vscp_omsg.class = VSCP_CLASS1_INFORMATION;
	vscp_omsg.type = VSCP_TYPE_INFORMATION_NODE_HEARTBEAT;
	vscp_omsg.data[ 0 ] = 0;			
	vscp_omsg.data[ 1 ] = zone;
	vscp_omsg.data[ 2 ] = subzone;			
				
	// send the message
	vscp_sendEvent();
}

///////////////////////////////////////////////////////////////////////////////
// vscp_handleHeartbeat
//

void vscp_handleHeartbeat( void )
{
	if ( ( 5 == ( vscp_imsg.flags & 0x0f ) ) && 
			( vscp_getSegmentCRC() != vscp_imsg.data[ 0 ] ) ) {

		// Stored CRC are different than received
		// We must be on a different segment
		vscp_setSegmentCRC( vscp_imsg.data[ 0 ] );
		
		// Introduce ourself in the proper way and start from the beginning
		vscp_nickname = VSCP_ADDRESS_FREE;
		vscp_writeNicknamePermanent( VSCP_ADDRESS_FREE );
		vscp_node_state = VSCP_STATE_INIT;

	}		
}

///////////////////////////////////////////////////////////////////////////////
// vscp_handleSetNickname
//

void vscp_handleSetNickname( void )
{
	if ( ( 2 == ( vscp_imsg.flags & 0x0f ) ) && 
					( vscp_nickname == vscp_imsg.data[ 0 ] ) ) {

		// Yes, we are addressed
		vscp_nickname = vscp_imsg.data[ 1 ];
		vscp_writeNicknamePermanent( vscp_nickname );		
		vscp_setSegmentCRC( 0x40 );
	}	
}

///////////////////////////////////////////////////////////////////////////////
// vscp_handleDropNickname
//

void vscp_handleDropNickname( void )
{
	if ( ( 1 == ( vscp_imsg.flags & 0x0f ) ) && 
					( vscp_nickname == vscp_imsg.data[ 0 ] ) ) {
		
		// Yes, we are addressed
		vscp_nickname = VSCP_ADDRESS_FREE;
		vscp_writeNicknamePermanent( VSCP_ADDRESS_FREE );
		vscp_init();

	}
}

///////////////////////////////////////////////////////////////////////////////
// vscp_newNodeOnline
//

void vscp_newNodeOnline( void )
{
	if ( ( 1 == ( vscp_imsg.flags & 0x0f ) ) && 
					( vscp_nickname == vscp_imsg.data[ 0 ] ) ) {

		// This is a probe which use our nickname
		// we have to respond to tell the new node that
		// ths nickname is in use

		vscp_omsg.flags = VSCP_VALID_MSG;
		vscp_omsg.priority = VSCP_PRIORITY_HIGH;	
		vscp_omsg.class = VSCP_CLASS1_PROTOCOL;
		vscp_omsg.type = VSCP_TYPE_PROTOCOL_PROBE_ACK;		
		vscp_sendEvent();				

	}
}

///////////////////////////////////////////////////////////////////////////////
// vscp_doOneSecondWork
//

void vscp_doOneSecondWork( void )
{
    ++vscp_second;

	if ( vscp_second > 59 ) {
		vscp_second = 0;
		vscp_minute++;
		
		// send periodic heartbeat
		if ( VSCP_STATE_ACTIVE == vscp_node_state  ) {
			vscp_sendHeartBeat( vscp_getZone(), 
								 vscp_getSubzone() );
		}
	}
	
	if ( vscp_minute > 59 ) {
		vscp_minute = 0;
		vscp_hour++;
	}
				
	if ( vscp_hour > 23 ) vscp_hour = 0;	
		
	if ( VSCP_STATE_ACTIVE == vscp_node_state  ) {
		vscp_guid_reset++;
		if ( (vscp_guid_reset & 0x0f) >= 2 ) {
			vscp_guid_reset = 0;	
		}
	}
	else {
	
	}		
}

///////////////////////////////////////////////////////////////////////////////
// vscp_readRegister
//

uint8_t vscp_readRegister( uint8_t reg )
{
	if ( reg >= 0x80 ) {
		return vscp_readStdReg( reg );
	}
	else {
		return vscp_readAppReg( reg );
	}		
}

///////////////////////////////////////////////////////////////////////////////
// vscp_readStdReg
//

uint8_t vscp_readStdReg( uint8_t reg )
{
	uint8_t rv = 0;
	
	if ( VSCP_REG_ALARMSTATUS == vscp_imsg.data[ 1 ] ) {

		// * * * Read alarm status register * * * 
		rv = vscp_alarmstatus;
		vscp_alarmstatus = 0x00;		// Reset alarm status

	}

	else if ( VSCP_REG_VSCP_MAJOR_VERSION == vscp_imsg.data[ 1 ] ) {

		// * * * VSCP Protocol Major Version * * * 
		rv = VSCP_MAJOR_VERSION;

	}

	else if ( VSCP_REG_VSCP_MINOR_VERSION == vscp_imsg.data[ 1 ] ) {
		
 		// * * * VSCP Protocol Minor Version * * * 
		rv = VSCP_MINOR_VERSION;

	}

	else if ( VSCP_REG_NODE_CONTROL == vscp_imsg.data[ 1 ] ) {
		
 		// * * * Reserved * * * 
		rv = 0;

	}

	else if ( VSCP_REG_FIRMWARE_MAJOR_VERSION == vscp_imsg.data[ 1 ] ) {
		
 		// * * * Get firmware Major version * * * 
		rv = vscp_getMajorVersion();	

	}

	else if ( VSCP_REG_FIRMWARE_MINOR_VERSION == vscp_imsg.data[ 1 ] ) {
		
 		// * * * Get firmware Minor version * * * 
		rv = vscp_getMinorVersion();	

	}

	else if ( VSCP_REG_FIRMWARE_SUB_MINOR_VERSION == vscp_imsg.data[ 1 ] ) {
		
 		// * * * Get firmware Sub Minor version * * * 
		rv = vscp_getSubMinorVersion();	

	}

	else if ( vscp_imsg.data[ 1 ] < VSCP_REG_MANUFACTUR_ID0 ) {

		// * * * Read from persitant locations * * * 
		rv = vscp_getUserID( vscp_imsg.data[ 1 ] - VSCP_REG_USERID0 );	

	}

	else if ( ( vscp_imsg.data[ 1 ] > VSCP_REG_USERID4 )  &&  
				( vscp_imsg.data[ 1 ] < VSCP_REG_NICKNAME_ID ) ) {
		
 		// * * * Manufacturer ID information * * * 
		rv = vscp_getManufacturerId( vscp_imsg.data[ 1 ] - VSCP_REG_MANUFACTUR_ID0 ); 			

	}

	else if ( VSCP_REG_NICKNAME_ID == vscp_imsg.data[ 1 ] ) {

		// * * * nickname id * * * 
		rv = vscp_nickname;

	}

	else if ( VSCP_REG_PAGE_SELECT_LSB == vscp_imsg.data[ 1 ] ) {
		
 		// * * * Page select LSB * * * 
		rv = ( vscp_page_select & 0xff );	

	}	

	else if ( VSCP_REG_PAGE_SELECT_MSB == vscp_imsg.data[ 1 ] ) {
		
 		// * * * Page select MSB * * * 
		rv = ( vscp_page_select >> 8 ) & 0xff;	

	}
	
	else if ( VSCP_REG_BOOT_LOADER_ALGORITHM == vscp_imsg.data[ 1 ] ) {
		// * * * Boot loader algorithm * * *
		rv = vscp_getBootLoaderAlgorithm();
	}
	
	else if ( VSCP_REG_BUFFER_SIZE == vscp_imsg.data[ 1 ] ) {
		// * * * Buffer size * * *
		rv = vscp_getBufferSize();	
	}
    
    else if ( VSCP_REG_PAGES_USED == vscp_imsg.data[ 1 ] ) {
		// * * * Register Pages Used * * *
		rv = vscp_getRegisterPagesUsed();	
	}
			
	else if ( ( vscp_imsg.data[ 1 ] > ( VSCP_REG_GUID - 1 ) ) && 
				( vscp_imsg.data[ 1 ] < VSCP_REG_DEVICE_URL )  ) {

		// * * * GUID * * * 
		rv = vscp_getGUID( vscp_imsg.data[ 1 ] - VSCP_REG_GUID );		

	}

	else {

		// * * * The device URL * * * 
		rv = vscp_getMDF_URL( vscp_imsg.data[ 1 ] - VSCP_REG_DEVICE_URL );	

	}
	
	return rv;
}

///////////////////////////////////////////////////////////////////////////////
// vscp_writeRegister
//

uint8_t vscp_writeRegister( uint8_t reg, uint8_t value )
{
	if ( reg >= 0x80 ) {
		return vscp_writeStdReg( reg, value );
	}
	else {
		return vscp_writeAppReg( reg, value );
	}		
}

///////////////////////////////////////////////////////////////////////////////
// vscp_writeStdReg
//

uint8_t vscp_writeStdReg( uint8_t reg, uint8_t value )
{
	uint8_t rv = ~value;

	if ( ( vscp_imsg.data[ 1 ] > ( VSCP_REG_VSCP_MINOR_VERSION + 1 ) ) && 
		( vscp_imsg.data[ 1 ] < VSCP_REG_MANUFACTUR_ID0 ) ) {
	 
		// * * * User Client ID * * *
		vscp_setUserID( ( vscp_imsg.data[ 1 ] - VSCP_REG_USERID0 ), vscp_imsg.data[ 2 ] );		
		rv = vscp_getUserID( ( vscp_imsg.data[ 1 ] - VSCP_REG_USERID0 ) );
	
	}

	else if ( VSCP_REG_PAGE_SELECT_MSB == vscp_imsg.data[ 1 ] ) {

		// * * * Page select register MSB * * *
		vscp_page_select = ( vscp_page_select & 0xff00 ) | ( (uint16_t)vscp_imsg.data[ 2 ] << 8 );
		rv = ( vscp_page_select >> 8 ) & 0xff;
	}
	
	else if ( VSCP_REG_PAGE_SELECT_LSB == vscp_imsg.data[ 1 ] ) {

		// * * * Page select register LSB * * *
		vscp_page_select = ( vscp_page_select & 0xff ) | vscp_imsg.data[ 2 ];
		rv = ( vscp_page_select & 0xff );
	}	

#ifdef ENABLE_WRITE_2PROTECTED_LOCATIONS

	// Write manufacturer id configuration information
	else if ( ( vscp_imsg.data[ 1 ] > VSCP_REG_USERID4 )  &&  ( vscp_imsg.data[ 1 ] < VSCP_REG_NICKNAME_ID ) ) {
		// page select must be 0xffff for writes to be possible
		if ( ( 0xff != ( ( vscp_page_select >> 8 ) & 0xff ) ) || 
				( 0xff != ( vscp_page_select & 0xff ) ) ) {
			// return complement to indicate error
			rv = ~vscp_imsg.data[ 2 ];	
		}
		else {
			// Write
			vscp_setManufacturerId( vscp_imsg.data[ 1 ] - VSCP_REG_MANUFACTUR_ID0, vscp_imsg.data[ 2 ] );
			rv = vscp_getManufacturerId( vscp_imsg.data[ 1 ] - VSCP_REG_MANUFACTUR_ID0 );
		}
	}
	// Write GUID configuration information
	else if ( ( vscp_imsg.data[ 1 ] > ( VSCP_REG_GUID - 1 ) ) && ( vscp_imsg.data[ 1 ] < VSCP_REG_DEVICE_URL )  ) {
		// page must be 0xffff for writes to be possible
		if ( ( 0xff != ( ( vscp_page_select >> 8 ) & 0xff ) ) || 
				( 0xff != ( vscp_page_select & 0xff ) ) )  {
			// return complement to indicate error
			rv = ~vscp_imsg.data[ 2 ];	
		}
		else {
			vscp_setGUID( vscp_imsg.data[ 1 ] - VSCP_REG_GUID, vscp_imsg.data[ 2 ] );
			rv = vscp_getGUID( vscp_imsg.data[ 1 ] - VSCP_REG_GUID );
		}
	}
	
#endif

	else {
		// return complement to indicate error
		rv = ~value;		
	}

	return rv;
}

///////////////////////////////////////////////////////////////////////////////
// vscp_writeStdReg
//

void vscp_handleProtocolEvent( void )
{

	if ( VSCP_CLASS1_PROTOCOL == vscp_imsg.class ) {
						
						
		switch( vscp_imsg.type ) {

			case VSCP_TYPE_PROTOCOL_SEGCTRL_HEARTBEAT:
			
				vscp_handleHeartbeat();
				break;

			case VSCP_TYPE_PROTOCOL_NEW_NODE_ONLINE:
			
				vscp_newNodeOnline();
				if(node_list[vscp_imsg.data[0]] == 0) {
					++active_node;
					node_list[vscp_imsg.data[0]] = TIMEOUT_HEARTBEAT;
				}
				printf("Connected NODE :: Nickname [ %x ]\n", vscp_imsg.data[0]);
				break;
							
			case VSCP_TYPE_PROTOCOL_SET_NICKNAME:
			
				vscp_handleSetNickname();
				break;
							
			case VSCP_TYPE_PROTOCOL_DROP_NICKNAME:
			
				vscp_handleDropNickname();
				break;

			case VSCP_TYPE_PROTOCOL_READ_REGISTER:
			
				if ( ( 2 == ( vscp_imsg.flags & 0x0f ) ) && 
					 ( vscp_nickname == vscp_imsg.data[ 0 ] ) ) {

					if ( vscp_imsg.data[ 1 ] < 0x80 )  {
										
						vscp_omsg.priority = VSCP_PRIORITY_MEDIUM;
						vscp_omsg.flags = VSCP_VALID_MSG + 2;
						vscp_omsg.class = VSCP_CLASS1_PROTOCOL;
						vscp_omsg.type = VSCP_TYPE_PROTOCOL_RW_RESPONSE;
										
						// Register to read
						vscp_omsg.data[ 0 ] = vscp_imsg.data[ 1 ]; 
										
						// Read application specific register
						vscp_omsg.data[ 1 ] = vscp_readAppReg( vscp_imsg.data[ 1 ] );
										
						// Send reply data
						vscp_sendEvent();
					}
					else {
						vscp_omsg.priority = VSCP_PRIORITY_NORMAL;
						vscp_omsg.flags = VSCP_VALID_MSG + 2;
						vscp_omsg.class = VSCP_CLASS1_PROTOCOL;
						vscp_omsg.type = VSCP_TYPE_PROTOCOL_RW_RESPONSE;
	
						// Register to read
						vscp_omsg.data[ 0 ] =  vscp_imsg.data[ 1 ];
										
						// Read VSCP register
						vscp_omsg.data[ 1 ] = 
									vscp_readStdReg( vscp_imsg.data[ 1 ] );
										
						// Send event
						vscp_sendEvent();
					}
				}
				break;

			case VSCP_TYPE_PROTOCOL_WRITE_REGISTER:
			
				if ( ( 3 == ( vscp_imsg.flags & 0x0f ) ) && 
					 ( vscp_nickname == vscp_imsg.data[ 0 ] ) ) {
						
					if ( vscp_imsg.data[ 1 ] < 0x80 ) {
										
						vscp_omsg.priority = VSCP_PRIORITY_MEDIUM;
						vscp_omsg.flags = VSCP_VALID_MSG + 2;
						vscp_omsg.class = VSCP_CLASS1_PROTOCOL;
						vscp_omsg.type = VSCP_TYPE_PROTOCOL_RW_RESPONSE;

						// Register read
						vscp_omsg.data[ 0 ] = vscp_imsg.data[ 1 ];
										
						// Write application specific register
						vscp_omsg.data[ 1 ] = 
						vscp_writeAppReg( vscp_imsg.data[ 1 ], vscp_imsg.data[ 2 ] );
										
						// Send reply
						vscp_sendEvent();
					}
					else {
						vscp_omsg.priority = VSCP_PRIORITY_MEDIUM;
						vscp_omsg.flags = VSCP_VALID_MSG + 2;
						vscp_omsg.class = VSCP_CLASS1_PROTOCOL;
						vscp_omsg.type = VSCP_TYPE_PROTOCOL_RW_RESPONSE;
						vscp_omsg.data[ 0 ] = vscp_nickname; 
										
						// Register read
						vscp_omsg.data[ 0 ] = vscp_imsg.data[ 1 ];
										
						// Write VSCP register
						vscp_omsg.data[ 1 ] =
						vscp_writeStdReg( vscp_imsg.data[ 1 ], vscp_imsg.data[ 2 ] );
										
						// Write event
						vscp_sendEvent();
					}
				}
				break;
								
			case VSCP_TYPE_PROTOCOL_ENTER_BOOT_LOADER:
			
				if ( ( vscp_nickname == vscp_imsg.data[ 0 ] ) &&
					 	( 1 == vscp_imsg.data[ 1 ] ) &&
					 	( vscp_getGUID( 0 ) == vscp_imsg.data[ 2 ] ) &&
					 	( vscp_getGUID( 3 ) == vscp_imsg.data[ 3 ] ) &&
					 	( vscp_getGUID( 5 ) == vscp_imsg.data[ 4 ] ) &&
					 	( vscp_getGUID( 7 ) == vscp_imsg.data[ 5 ] ) &&
					 	( ( vscp_page_select >> 8 ) == vscp_imsg.data[ 6 ] ) &&
					 	( ( vscp_page_select & 0xff ) == vscp_imsg.data[ 7 ] ) ) {	
						 		
					vscp_goBootloaderMode();	 											
		
				}
				break;
								
			case VSCP_TYPE_PROTOCOL_RESET_DEVICE:
			
				switch ( vscp_imsg.data[ 0 ] >> 4 ) {
									
					case 0:
						if ( ( vscp_getGUID( 0 ) == vscp_imsg.data[ 1 ] ) &&
							 ( vscp_getGUID( 1 ) == vscp_imsg.data[ 2 ] ) &&
							 ( vscp_getGUID( 2 ) == vscp_imsg.data[ 3 ] ) &&
							 ( vscp_getGUID( 3 ) == vscp_imsg.data[ 4 ] ) ) {
							vscp_guid_reset |= 0x10;
						}	
						break;
										
					case 1:
						if ( ( vscp_getGUID( 4 ) == vscp_imsg.data[ 1 ] ) &&
							 ( vscp_getGUID( 5 ) == vscp_imsg.data[ 2 ] ) &&
							 ( vscp_getGUID( 6 ) == vscp_imsg.data[ 3 ] ) &&
							 ( vscp_getGUID( 7 ) == vscp_imsg.data[ 4 ] ) ) {
							vscp_guid_reset |= 0x20;
						}
						break;
										
					case 2:
						if ( ( vscp_getGUID( 8 ) == vscp_imsg.data[ 1 ] ) &&
							 ( vscp_getGUID( 9 ) == vscp_imsg.data[ 2 ] ) &&
						     ( vscp_getGUID( 10 ) == vscp_imsg.data[ 3 ] ) &&
							 ( vscp_getGUID( 11 ) == vscp_imsg.data[ 4 ] ) ) {
							vscp_guid_reset |= 0x40;
						}
						break;
										
					case 3:
						if ( ( vscp_getGUID( 12 ) == vscp_imsg.data[ 1 ] ) &&
							 ( vscp_getGUID( 13 ) == vscp_imsg.data[ 2 ] ) &&
							 ( vscp_getGUID( 14 ) == vscp_imsg.data[ 3 ] ) &&
							 ( vscp_getGUID( 15 ) == vscp_imsg.data[ 4 ] ) ) {
							vscp_guid_reset |= 0x80;
						}
						break;
										
					default:
						vscp_guid_reset = 0;	
						break;				
				}
									
				if ( 0xf0 == (vscp_guid_reset & 0xf0) ) {
					// Do a reset
					vscp_init();
				}	
				break;
								
			case VSCP_TYPE_PROTOCOL_PAGE_READ:
			
				if ( vscp_nickname == vscp_imsg.data[ 0 ] ) {
									
					uint8_t i;
					uint8_t pos = 0;
					uint8_t bSent = TRUE;
										 
					for ( i = vscp_imsg.data[ 1 ]; 
							i < ( vscp_imsg.data[ 1 ] + vscp_imsg.data[ 2 ] + 1 ); 
							i++ ) {
											   	
						vscp_omsg.flags = VSCP_VALID_MSG + ( pos % 7 ) + 1;
						vscp_omsg.data[ ( pos % 7 ) + 1 ] = 
													vscp_readAppReg( i );
						bSent = FALSE;	   	
											  
						if ( pos && ( 0 == ( pos % 7 ) ) ) {	   
							
							vscp_omsg.priority = VSCP_PRIORITY_NORMAL;	
							vscp_omsg.class = VSCP_CLASS1_PROTOCOL;
							vscp_omsg.type = VSCP_TYPE_PROTOCOL_RW_PAGE_RESPONSE;
							vscp_omsg.data[ 0 ] = pos/7;	// index
											
							vscp_omsg.flags = VSCP_VALID_MSG + 7; // Count = 7			
						
							// send the event
							vscp_sendEvent();
											
							bSent = TRUE;
						}
										
						pos++;
															
					}
									
					// Send any pending event
					if ( bSent ) {
						vscp_omsg.priority = VSCP_PRIORITY_NORMAL;	
						vscp_omsg.class = VSCP_CLASS1_PROTOCOL;
						vscp_omsg.type = VSCP_TYPE_PROTOCOL_RW_PAGE_RESPONSE;
						vscp_omsg.data[ 0 ] = pos/7;	// index
																	
						// send the event
						vscp_sendEvent();	
					}	
										
				}	
				break;	
								
			case VSCP_TYPE_PROTOCOL_PAGE_WRITE:
			
				if ( vscp_nickname == vscp_imsg.data[ 0 ] ) {
					uint8_t i;
					uint8_t pos = vscp_imsg.data[ 1 ];
									
					for ( i = 0; (i < ( vscp_imsg.flags & 0x0f ) - 2); i++ ) {
						// Write VSCP register
						vscp_omsg.data[ i + 1 ] =
										vscp_writeStdReg( vscp_imsg.data[ 1 ] + i,
										vscp_imsg.data[ 2 + i ] );			
					}
									
					vscp_omsg.priority = VSCP_PRIORITY_NORMAL;	
					vscp_omsg.class = VSCP_CLASS1_PROTOCOL;
					vscp_omsg.type = VSCP_TYPE_PROTOCOL_RW_PAGE_RESPONSE;
					vscp_omsg.data[ 0 ] = 0;	// index
					vscp_omsg.flags = VSCP_VALID_MSG + 
												( vscp_imsg.flags & 0x0f ) - 2;
																	
					// send the event
					vscp_sendEvent();
																				
				}
				break;	
								
			case VSCP_TYPE_PROTOCOL_INCREMENT_REGISTER:
			
				if ( vscp_nickname == vscp_imsg.data[ 0 ] ) {
									
					vscp_omsg.priority = VSCP_PRIORITY_NORMAL;
					vscp_omsg.flags = VSCP_VALID_MSG + 2;
					vscp_omsg.class = VSCP_CLASS1_PROTOCOL;
					vscp_omsg.type = VSCP_TYPE_PROTOCOL_RW_RESPONSE;
										
					vscp_omsg.data[ 0 ] = vscp_imsg.data[ 1 ];
					vscp_omsg.data[ 1 ] = vscp_writeAppReg( 
													vscp_imsg.data[ 1 ],
													vscp_readAppReg( vscp_imsg.data[ 1 ] ) + 1 );
									
					// send the event
					vscp_sendEvent();										
				}
				break;
								
			case VSCP_TYPE_PROTOCOL_DECREMENT_REGISTER:
			
				if ( vscp_nickname == vscp_imsg.data[ 0 ] ) {
									
					vscp_omsg.priority = VSCP_PRIORITY_NORMAL;
					vscp_omsg.flags = VSCP_VALID_MSG + 2;
					vscp_omsg.class = VSCP_CLASS1_PROTOCOL;
					vscp_omsg.type = VSCP_TYPE_PROTOCOL_RW_RESPONSE;
										
					vscp_omsg.data[ 0 ] = vscp_imsg.data[ 1 ];
					vscp_omsg.data[ 1 ] = vscp_writeAppReg( 
														vscp_imsg.data[ 1 ],
														vscp_readAppReg( vscp_imsg.data[ 1 ] ) - 1 );
									
					// send the message
					vscp_sendEvent();	
				}
				break;
								
			case VSCP_TYPE_PROTOCOL_WHO_IS_THERE:
			
				if ( ( vscp_nickname == vscp_imsg.data[ 0 ] ) ||
					 ( 0xff == vscp_imsg.data[ 0 ] ) ) {
											
					uint8_t i, j, k = 0;
				
					// Send data
					
					vscp_omsg.priority = VSCP_PRIORITY_NORMAL;
					vscp_omsg.flags = VSCP_VALID_MSG + 8;
					vscp_omsg.class = VSCP_CLASS1_PROTOCOL;
					vscp_omsg.type = VSCP_TYPE_PROTOCOL_WHO_IS_THERE_RESPONSE;
					
					for(i=0 ; i < 3; i++)			// fill up with GUID
					{
					     vscp_omsg.data[0] = i;	
					     
					     for(j = 1;  j < 8 ; j++)
					     {
						 vscp_omsg.data[j] = vscp_getGUID(15 - k++);
						 if (k > 16)
						      break;
					     }
					     
					      if (k > 16)
						      break;
					      
					     vscp_sendEvent();
					}
					
					for(j = 0; j < 5; j++)		// fillup previous message with MDF
					{
					     if(vscp_getMDF_URL(j)>0)
						  vscp_omsg.data[3 +j] = vscp_getMDF_URL(j);
					     else 
						  vscp_omsg.data[3 +j] = 0;
					}
					
					vscp_sendEvent();

					k = 5;					// start offset
					for(i=3 ; i < 7; i++)			// fill up with the rest of GUID
					{
					     vscp_omsg.data[0] = i;	
					     
					     for(j = 1;  j < 8 ; j++)
					     {
						 vscp_omsg.data[j] = vscp_getMDF_URL(k++);
					     }
					     vscp_sendEvent();
					}

				}
				break;
								
			case VSCP_TYPE_PROTOCOL_GET_MATRIX_INFO:
			
				if ( vscp_nickname == vscp_imsg.data[ 0 ] ) {
                    vscp_omsg.priority = VSCP_PRIORITY_NORMAL;
                    vscp_omsg.flags = VSCP_VALID_MSG + 7;
                    vscp_omsg.class = VSCP_CLASS1_PROTOCOL;
                    vscp_omsg.type = VSCP_TYPE_PROTOCOL_GET_MATRIX_INFO_RESPONSE;
										
                    vscp_getMatrixInfo( (char *)vscp_omsg.data );
								
                    // send the event
                    vscp_sendEvent();
                }
				break;
								
			case VSCP_TYPE_PROTOCOL_GET_EMBEDDED_MDF:
			
				vscp_getEmbeddedMdfInfo();
				break;
								
			case VSCP_TYPE_PROTOCOL_EXTENDED_PAGE_READ:
			
				if ( vscp_nickname == vscp_imsg.data[ 0 ] ) {
					uint8_t i;
					uint16_t page_save;
					uint8_t page_msb = vscp_imsg.data[ 1 ];
					uint8_t page_lsb = vscp_imsg.data[ 2 ];
									
					// Save the current page
					page_save = vscp_page_select;
									
					// Check for valid count
					if ( vscp_imsg.data[ 3 ] > 6 ) vscp_imsg.data[ 3 ] = 6;
									
					vscp_omsg.priority = VSCP_PRIORITY_NORMAL;
					vscp_omsg.flags = VSCP_VALID_MSG + 3 + vscp_imsg.data[ 3 ];
					vscp_omsg.class = VSCP_CLASS1_PROTOCOL;
					vscp_omsg.type = VSCP_TYPE_PROTOCOL_EXTENDED_PAGE_RESPONSE;
									
					for ( i=vscp_imsg.data[ 2 ];
							i < ( vscp_imsg.data[ 2 ] + vscp_imsg.data[ 3 ] );
							i++ ) {
						vscp_omsg.data[ 3 + ( i - vscp_imsg.data[ 2 ] ) ] = 
											vscp_readRegister( i ); 						
					}
									
					// Restore the saved page
					vscp_page_select = page_save;
									
					// send the event
					vscp_sendEvent();
										
				}	
				break;
								
			case VSCP_TYPE_PROTOCOL_EXTENDED_PAGE_WRITE:
			
				if ( vscp_nickname == vscp_imsg.data[ 0 ] ) {
					uint8_t i;
					uint16_t page_save;
					uint8_t page_msb = vscp_imsg.data[ 1 ];
					uint8_t page_lsb = vscp_imsg.data[ 2 ];
					
					// Save the current page
					page_save = vscp_page_select;
					
					vscp_omsg.priority = VSCP_PRIORITY_NORMAL;
					vscp_omsg.flags = VSCP_VALID_MSG + 3 + vscp_imsg.data[ 3 ];
					vscp_omsg.class = VSCP_CLASS1_PROTOCOL;
					vscp_omsg.type = VSCP_TYPE_PROTOCOL_EXTENDED_PAGE_RESPONSE;
					
					for ( i=vscp_imsg.data[ 2 ];
							i < ( vscp_imsg.data[ 2 ] + vscp_imsg.data[ 3 ] );
							i++ ) {
						vscp_omsg.data[ 3 + 
									( i - vscp_imsg.data[ 2 ] ) ] = 
										vscp_writeRegister( i, 
															vscp_imsg.data[ 4 + 
															( i - vscp_imsg.data[ 2 ] ) ] ); 						
					}

					// Restore the saved page
					vscp_page_select = page_save;
					
					// send the event
					vscp_sendEvent();
										
				}
				break;
                

			default:
				// Do work load
				break;
				
		} // switch							
					
	}	// CLASS1.PROTOCOL event
	
//----------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------
//moje modyfikacje zaczynaja sie tu!
//----------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------


//reakcja na ramke klasy 20 - INFORMATION
//Type 3 - przycisk wlaczony
//Type 4 - przycisk wylaczony
	if(VSCP_CLASS1_INFORMATION == vscp_imsg.class) {
		//oszukanie w przypadku danych z VSCP_WORKS
		//do wykomentowania vscp_imsg_oaddr...
		vscp_imsg.oaddr = 4; //---- VSCP WORKS  //VSCP_WORKS adres 0x04
		switch(vscp_imsg.type) {
			case VSCP_TYPE_INFORMATION_ON:
				node_light[vscp_imsg.oaddr] = 1;
				break;
				
			case VSCP_TYPE_INFORMATION_OFF:
				node_light[vscp_imsg.oaddr] = 0;
				break;
				
			case VSCP_TYPE_INFORMATION_NODE_HEARTBEAT:
				if(node_list[vscp_imsg.data[0]] != 0)
					node_list[vscp_imsg.data[0]] = TIMEOUT_HEARTBEAT;
				break;
				
			default:
				// Do work load
				break;
		}
	}	// CLASS20.INFORMATION event
	
	
//interpretacja wiadomosci class = 10 - MEASUREMENT
//type 6 - TEMPERATURE
//type 35 - HUMIDITY
	
	if(VSCP_CLASS1_MEASUREMENT == vscp_imsg.class) {
		//uint8_t x;
		vscp_imsg.oaddr = 4; //---- VSCP WORKS
		
		switch(vscp_imsg.type) {
			case VSCP_TYPE_MEASUREMENT_TEMPERATURE:
				if((vscp_imsg.flags & 0x0F) == 3) {
					//for(x=0; x<2; x++) ((uint8_t*)&node_temperature[vscp_imsg.oaddr])[x] = vscp_imsg.data[x+1];
					//do zakomentowania
					node_temperature[vscp_imsg.oaddr] = 2472; //wypelniam ramke przypadkowymi danym
					node_unit[vscp_imsg.oaddr] = (vscp_imsg.data[0] & 0x0F);
					
					//printf("temperature: %d  temp = %d\n", node_unit[vscp_imsg.oaddr], node_temperature[vscp_imsg.oaddr]);
				}
				break;
			case VSCP_TYPE_MEASUREMENT_HUMIDITY:
				if((vscp_imsg.flags & 0x0F) == 2) {		
					//for(x=0; x<2; x++) ((uint8_t*)&node_humidity[vscp_imsg.oaddr])[x] = vscp_imsg.data[x];
					//do zakomentowania
					node_humidity[vscp_imsg.oaddr] = 7024; //wypelniam wilgotnosc
					//printf("humidity\n");	
				}
				break;
			default:
				// Do work load
				break;
		}
	}	// CLASS10.MEASUREMENT event	
}


///////////////////////////////////////////////////////////////////////////////
// vscp_sendEvent
//

int8_t vscp_sendEvent( void )
{
    int8_t rv;
    
    if ( !( rv = sendVSCPFrame( vscp_omsg.class, 
						            vscp_omsg.type, 
						            vscp_nickname,
						            vscp_omsg.priority,
						            ( vscp_omsg.flags & 0x0f ), 
						            vscp_omsg.data ) ) ) {
        vscp_errorcnt++;    
    }    
    
    return rv;
}

///////////////////////////////////////////////////////////////////////////////
// vscp_getEvent
//

int8_t vscp_getEvent( void )
{
	int8_t rv;


	// Dont read in new message if there already is a message
	// in the input buffer. We return TRUE though to indicate there is
	// a valid event.
	if ( vscp_imsg.flags & VSCP_VALID_MSG ) return TRUE;
	
	
	if ( ( rv = getVSCPFrame( &vscp_imsg.class, 
						        &vscp_imsg.type, 
						        &vscp_imsg.oaddr, 
						        &vscp_imsg.priority, 
						        &vscp_imsg.flags, 
						        vscp_imsg.data ) ) ) {
    						        
        vscp_imsg.flags |= VSCP_VALID_MSG; 
      
        printf("RS232 -> Event NODE :: [ %x ]\t[ CLASS = %d ] [ TYPE = %d ]\n", vscp_imsg.oaddr, vscp_imsg.class, vscp_imsg.type);
    }				
	
	return rv;
	
}

//implementacja odpowiednich funkcji niezbednych do prawidlowego funkcjonowania wezla
/*!
    Get a VSCP frame frame
    @param pvscpclass Pointer to variable that will get VSCP class.
    @param pvscptype Ponter to variable which will get VSCP type.
    @param pNodeId Pointer to variable which will get nodeid.
	@param pPriority Pointer to variable which will get priority (0-7).
    @param pSize Pointer to variable that will get datasize.
    @param pData pinter to array that will get event data.
    @return TRUE on success.
*/
int8_t getVSCPFrame( uint16_t *pvscpclass,
                        uint8_t *pvscptype,
                        uint8_t *pNodeId,
                        uint8_t *pPriority,
                        uint8_t *pSize,
	                uint8_t *pData ) {
  uint8_t i = 0;
  uint32_t hdr = 0;
  for(i=0; i<4; i++) {
    ((uint8_t*)&hdr)[3-i] = buf[3+i];
  }
  *pvscptype = (uint8_t)((hdr>>8) & 0xFF);
  *pvscpclass = (uint16_t)((hdr>>16) & 0xFF);
  *pPriority = (uint8_t)(0x07 && (hdr>>26));
  *pNodeId = (uint8_t)(hdr & 0xFF);
  *pSize = (buf[2] & 0x0F);
  memcpy(pData, &buf[7], *pSize);		
  
//  uint8_t x;
//  for(x=0; x<sizeof(buf); x++) {
		//printf("%d :: %x\n", x, buf[x]);
  //}
  

  if(len-9 != *pSize) return FALSE;

//to odkomentowac
//zerowaniu w przypadku transmisji HEART_BREAT w przypadku 
//  if(node_list[*pNodeId] > 0)
	//node_list[*pNodeId] = TIMEOUT_HEARTBEAT;

//spreparowane na potrzeby wspolpracy z VSCP - zakomentowa
  if(node_list[4] > 0) 
  	node_list[4] = TIMEOUT_HEARTBEAT;

return TRUE;
}

c/*!
    Send a VSCP frame
    @param vscpclass VSCP class for event.
    @param vscptype VSCP type for event.
	@param nodeid Nodeid for originating node.
	@param priority Priotity for event.
    @param size Size of data portion.
    @param pData Pointer to event data.
    @return TRUE on success.
*/
int8_t sendVSCPFrame( uint16_t vscpclass,
                        uint8_t vscptype,
                        uint8_t nodeid,
                        uint8_t priority,
                        uint8_t size,
                        uint8_t *pData ) {
	uint8_t i = 0;
	uint8_t x;
	uint8_t buff[BUF_SIZE];
	uint32_t hdr = 0;

	hdr = (uint32_t)(priority << 26) | (uint32_t)(vscpclass << 16) | (vscptype << 8) | nodeid;
	
	buff[i++] = DLE;
	buff[i++] = STX;
	buff[i++] = 0x80 | size;

	buff[i++] = ((uint8_t*)&hdr)[3];
	buff[i++] = ((uint8_t*)&hdr)[2];
	buff[i++] = ((uint8_t*)&hdr)[1];
	buff[i++] = ((uint8_t*)&hdr)[0];

	for(x=0; x<size; x++)
		buff[i++] = pData[x];

	buff[i++] = DLE;
	buff[i] = ETX;

	while(pthread_mutex_trylock(&serial_mutex));
	if(write(serial_fd, buff, size+9) != -1) {
	  pthread_mutex_unlock(&serial_mutex);
	  return TRUE;
	}
	
	pthread_mutex_unlock(&serial_mutex);
	return FALSE;
 }


/*!
	The following methods must be defined
	in the application and should return firmware version
	information
*/
uint8_t vscp_getMajorVersion( void ) { 
	fseek(eeprom_fd, EEPROM_MAJOR_VERSION, SEEK_SET);
	return fgetc(eeprom_fd);
}

uint8_t vscp_getMinorVersion( void ) {
	fseek(eeprom_fd, EEPROM_MINOR_VERSION, SEEK_SET);
	return fgetc(eeprom_fd);
}

uint8_t vscp_getSubMinorVersion( void ) { 
	fseek(eeprom_fd, EEPROM_SUBMINOR_VERSION, SEEK_SET);
	return fgetc(eeprom_fd);
}

/*!
	Get GUID from permament storage
*/
uint8_t vscp_getGUID( uint8_t idx ) { 
	fseek(eeprom_fd, EEPROM_GUID+idx, SEEK_SET);
	return fgetc(eeprom_fd);
}

void vscp_setGUID( uint8_t idx, uint8_t data ) {
	fseek(eeprom_fd, EEPROM_GUID+idx, SEEK_SET);
	fputc(data, eeprom_fd);
}

/*!
	User ID 0 idx=0
	User ID 1 idx=1
	User ID 2 idx=2
	User ID 3 idx=3
*/
uint8_t vscp_getUserID( uint8_t idx ) {
	fseek(eeprom_fd, EEPROM_USER_ID+idx, SEEK_SET);
	return fgetc(eeprom_fd);
}

void vscp_setUserID( uint8_t idx, uint8_t data ) {
	fseek(eeprom_fd, EEPROM_USER_ID+idx, SEEK_SET);
	fputc(data, eeprom_fd);
	fflush(eeprom_fd);
	return;
}

/*!
	Handle manufacturer id.

	Not that both main and sub id are fetched here
		Manufacturer device ID byte 0 - idx=0
		Manufacturer device ID byte 1 - idx=1
		Manufacturer device ID byte 2 - idx=2
		Manufacturer device ID byte 3 - idx=3
		Manufacturer device sub ID byte 0 - idx=4
		Manufacturer device sub ID byte 1 - idx=5
		Manufacturer device sub ID byte 2 - idx=6
		Manufacturer device sub ID byte 3 - idx=7
*/
uint8_t vscp_getManufacturerId( uint8_t idx ) {
	fseek(eeprom_fd, EEPROM_MANUFACTURER+idx, SEEK_SET);
	return fgetc(eeprom_fd);
}

void vscp_setManufacturerId( uint8_t idx, uint8_t data ) {
	fseek(eeprom_fd, EEPROM_MANUFACTURER+idx, SEEK_SET);
	fputc(data, eeprom_fd);
	fflush(eeprom_fd);
}

/*!
	Get bootloader algorithm from permanent storage
*/
uint8_t vscp_getBootLoaderAlgorithm( void ) { return VSCP_BOOTLOADER_NONE; }

/*!
	Get buffer size
*/
uint8_t vscp_getBufferSize( void )  { return 0x00; }

/*!
	Get number of register pages used by app.
*/
uint8_t vscp_getRegisterPagesUsed( void ) { return 0x00; }

/*!
	Get URL from device from permanent storage
	index 0-15
*/
//nie ma pliku MDF
uint8_t vscp_getMDF_URL( uint8_t idx ) { return idx; }

/*!
	Fetch nickname from permanent storage
	@return read nickname.
*/
uint8_t vscp_readNicknamePermanent( void ) {
	fseek(eeprom_fd, EEPROM_NICKNAME, SEEK_SET);
	return fgetc(eeprom_fd);
}

/*!
	Write nickname to permanent storage
	@param nickname to write
*/
void vscp_writeNicknamePermanent( uint8_t nickname ) { 
	fseek(eeprom_fd, EEPROM_NICKNAME, SEEK_SET);
	fputc(nickname, eeprom_fd);
	fflush(eeprom_fd);
}

/*!
	Fetch segment CRC from permanent storage
*/
uint8_t vscp_getSegmentCRC( void ) { 
	fseek(eeprom_fd, EEPROM_CRC, SEEK_SET);
	return fgetc(eeprom_fd);
}

/*!
	Write segment CRC to permanent storage
*/
void vscp_setSegmentCRC( uint8_t crc ) { 
	fseek(eeprom_fd, EEPROM_CRC, SEEK_SET);
	fputc(crc, eeprom_fd);
	fflush(eeprom_fd);
}

/*!
	Write control byte permanent storage
*/
void vscp_setControlByte( uint8_t ctrl ) {
	fseek(eeprom_fd, EEPROM_CTRL_BYTE, SEEK_SET);
	fputc(ctrl, eeprom_fd);
	fflush(eeprom_fd);
}

/*!
 	Fetch control byte from permanent storage
*/
uint8_t vscp_getControlByte( void ) {
	fseek(eeprom_fd, EEPROM_CTRL_BYTE, SEEK_SET);
	return fgetc(eeprom_fd);
}

/*!
	Get page select bytes
		idx=0 - byte 0 MSB
		idx=1 - byte 1 LSB
*/
uint8_t vscp_getPageSelect( uint8_t idx ) { return 0; }

/*!
	Set page select registers
	@param idx 0 for LSB, 1 for MSB
	@param data Byte to set of page select registers
*/
void vscp_setPageSelect( uint8_t idx, uint8_t data ) { return; }

/*!
	Read application register (lower part)
	@param reg Register to read (<0x80)
	@return Register content or 0x00 for non valid register
*/
uint8_t vscp_readAppReg( uint8_t reg ) {
	fseek(eeprom_fd, EEPROM_APP_REGISTER+reg, SEEK_SET);
	return fgetc(eeprom_fd);
}

/*!
	Write application register (lower part)
	@param reg Register to read (<0x80)
	@param value Value to write to register.
	@return Register content or 0xff for non valid register
*/
uint8_t vscp_writeAppReg( uint8_t reg, uint8_t value ) { 
	fseek(eeprom_fd, EEPROM_APP_REGISTER+reg, SEEK_SET);
	fputc(value, eeprom_fd);
	fflush(eeprom_fd);
	fseek(eeprom_fd, EEPROM_APP_REGISTER+reg, SEEK_SET);
	return fgetc(eeprom_fd);
}

/*!
	Get DM matrix info
	The output message data structure should be filled with
	the following data by this routine.
	byte 0 - Number of DM rows. 0 if none.
	byte 1 - offset in register space.
u	byte 2 - start page MSB
	byte 3 - start page LSB
	byte 4 - End page MSB
	byte 5 - End page LSB
	byte 6 - Level II size of DM row (Just for Level II nodes).
*/
void vscp_getMatrixInfo( char *pData ) { return; }

/*!
	Get embedded MDF info
	If available this routine sends an embedded MDF file
	in several events. See specification CLASS1.PROTOCOL
	Type=35/36
*/
void vscp_getEmbeddedMdfInfo( void ) { return; }

/*!
	Go bootloader mode
	This routine force the system into bootloader mode according
	to the selected protocol.
*/
void vscp_goBootloaderMode( void ) { return; }

/*!
	Get Zone for device
	Just return zero if not used.
*/
 uint8_t vscp_getZone( void ) {
	 fseek(eeprom_fd, EEPROM_ZONE, SEEK_SET);
	 return fgetc(eeprom_fd);
 }

/*!
	Get Subzone for device
	Just return zero if not used.
*/
 uint8_t vscp_getSubzone( void ) {
	 fseek(eeprom_fd, EEPROM_SUBZONE, SEEK_SET);
	 return fgetc(eeprom_fd);
 }
