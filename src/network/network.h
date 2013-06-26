#ifndef NETWORK_H
#define NETWORK_H


#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <time.h>

#include "nrf24/nrf24.h"
#include "rnp.h"


#define true 1
#define false 0


#define ENABLE_REDUCE_MEMORY_FOOTPRINT 0


#define NET_OK                          ((uint8_t)0x00)
#define NETERR_INVALID_INPUT_POINTER    ((uint8_t)0x01)
#define NETERR_INVALID_OUTPUT_POINTER   ((uint8_t)0x02)
#define NETERR_QUEUE_FULL               ((uint8_t)0x03)
#define NETERR_QUEUE_EMPTY              ((uint8_t)0x04)
#define NETERR_INVALID_CONTEXT          ((uint8_t)0x05)
#define NETERR_PACKET_TOO_LONG          ((uint8_t)0x06)
#define NETERR_INVALID_ADDRESS          ((uint8_t)0x07)

/// The listen mode will be activated immediately.
#define NET_LISTEN_AUTO                 ((uint8_t)0x00)
/// The listen mode will be activated manually.
#define NET_LISTEN_MANUAL               ((uint8_t)0x01)


#if (ENABLE_REDUCE_MEMORY_FOOTPRINT == 0)
	#define FRAME_QUEUE_ENTRIES         0x02
#else
	#define FRAME_QUEUE_ENTRIES         0x01
#endif


#define NET_HMASK(addr) \
	( ( ( ( (addr) & 07) > 0 ) * 07) | \
	  ( ( ( (addr) & 070) > 0 ) * 077) | \
	  ( ( ( (addr) & 0700) > 0 ) * 0777) | \
	  ( ( ( (addr) & 07000) > 0 ) * 07777) | \
	  ( ( ( (addr) & 070000) > 0 ) * 077777) )

#define NET_RMASK(addr) \
	( ( ( ( (addr) & 0xF) != 0x3 ) * 0xF) | \
	  ( ( ( (addr) & 0xF0) != 0x30 ) * 0xFF) | \
	  ( ( ( (addr) & 0xF00) != 0x300 ) * 0xFFF) | \
	  ( ( ( (addr) & 0xF000) != 0x3000 ) * 0xFFFF) | \
	  ( ( ( (addr) & 0xF0000) != 0x30000 ) * 0xFFFFF) )

#define NET_FRAME_LENGTH                NRF24_MAX_FRAME_LENGTH

typedef uint32_t rf24_address_t;

typedef uint8_t bool;



typedef struct
{

	/**
	* Logical address of the node.
	*/
	uint16_t hostAddress;

	uint16_t hostAddressMask;

#if (ENABLE_DIRECT_SEND == 1)
	uint16_t unreachables[2];
#endif

	uint16_t initialized : 1;

	uint16_t listenAuto : 1;

	uint16_t nextFrame : 2;

	uint16_t rnpNextPacketId : 4;

	uint16_t rnpPacketMaxHop : 4;

	uint16_t rnpPacketOptions : 4;

	/** 
	 * Space to put the frame that will be sent/received over the air.
	 */
	uint8_t frameBuffer[NET_FRAME_LENGTH];

	/**
	* Space for a small set of frames that need to be delivered to the app layer
	*/
	uint8_t frameQueue[FRAME_QUEUE_ENTRIES * NET_FRAME_LENGTH];

	//nrf24_context_t *radio;

} network_context_t;


#include "ncmp.h"


uint8_t net_initialize(
	network_context_t *context,
	uint8_t listenMode );

uint8_t net_terminate(
	network_context_t *context );

uint8_t net_zeroconf(
	network_context_t *context,
	uint8_t channel );

uint8_t net_start(
	network_context_t *context,
	uint8_t channel,
	rnp_address_t address );

void net_update(
	network_context_t *context );

uint8_t net_receive(
	network_context_t *context,
	rnp_header_t *header,
	uint8_t *data,
	uint8_t *length );

uint8_t net_send(
	network_context_t *context,
	const rnp_header_t *header,
	const uint8_t *data );

uint8_t net_getNextHop(
	network_context_t *context,
	rnp_address_t destination,
	rnp_address_t *nextHop );

#if (ENABLE_DIRECT_SEND == 1)
uint8_t net_addUnreachable(
	network_context_t *context,
	uint16_t address );

bool net_isUnreachable(
	network_context_t *context,
	rnp_address_t address );

uint8_t net_updateUnreachables(
	network_context_t *context );

#endif  // (ENABLE_DIRECT_SEND == 1)

bool net_isDirectChild(
	network_context_t *context,
	rnp_address_t address );

bool net_isChild(
	network_context_t *context,
	rnp_address_t address );

uint8_t net_setAddress(
	network_context_t *context,
	rnp_address_t address );

bool net_isValidHostAddress(
	rnp_address_t address );

bool net_isValidRadioAddress(
	rf24_address_t address );

uint8_t net_getHostAddress(
	rf24_address_t radioAddress,
	rnp_address_t *hostAddress );

uint8_t net_getRadioAddress(
	rnp_address_t hostAddress,
	rf24_address_t *radioAddress );

uint32_t net_getMillis();

#endif  // NETWORK_H
