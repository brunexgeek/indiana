#include "network.h"


static rf24_address_t EMPTY_PHYS_ADDRESS = 0xE3300000;

static uint16_t BROADCAST_ADDRESS = 077777;

const rf24_address_t BROADCAST_PHYS_ADDRESS = 0xE333CCCC;

const uint8_t LOGICAL_DIGITS[] =
	{ 0x03, 0x0F, 0x0E, 0x06, 0x01, 0x09, 0x0B, 0x04 };

const uint8_t PHYSICAL_DIGITS[] =
	{ 0xFF, 0x04, 0xFF, 0x00, 0x07, 0xFF, 0x03, 0xFF,
	  0xFF, 0x05, 0xFF, 0x06, 0xFF, 0xFF, 0x02, 0x01 };



static uint8_t net_enqueue(
	network_context_t *context );

static uint8_t net_dequeue(
	network_context_t *context );

static uint8_t net_peek(
	network_context_t *context,
	rnp_header_t* header);

static uint8_t net_transmit(
	network_context_t *context );

static uint8_t net_write(
	network_context_t *context,
	rnp_address_t address );


uint8_t net_initialize(
	network_context_t *context )
{
	if (context == 0) return NETERR_INVALID_INPUT_POINTER;
	memset(context, 0, sizeof(network_context_t));

	context->nextFrame = 0;
	context->rnpNextPacketId = 1;
	context->rnpPacketMaxHop = 10;
	context->rnpPacketOptions = RNP_OPTION_ACK | RNP_OPTION_RETRANSMIT;

	nrf24_initialize();

	context->initialized = TRUE;

	return NET_OK;
}

uint8_t net_terminate(
	network_context_t *context )
{
	if (context == 0) return NETERR_INVALID_INPUT_POINTER;
	memset(context, 0, sizeof(network_context_t));

	nrf24_terminate();

	return NET_OK;
}


uint8_t net_zeroconf(
	network_context_t *context,
	uint8_t channel )
{
	uint8_t result;

	if (context == 0) return NETERR_INVALID_INPUT_POINTER;

#if (ENABLE_DIRECT_SEND == 1)
	memset(context->unreachables, 0, sizeof(context->unreachables));
#endif
	// setup the internal addresses as a broadcast
	result = net_setAddress(context, BROADCAST_ADDRESS);
	if (result != NET_OK) return result;
	// configure the radio
	nrf24_setChannel(channel);
	nrf24_setPayloadLength(NET_FRAME_LENGTH);
	// configure the radio pipelines and start listening
	nrf24_openReadingPipe(0, BROADCAST_ADDRESS);
	nrf24_startListening();
	// try to find some parent to adopt us
	while (1)
	{
		// broadcast the adoption anouncement
		ncmp_send(context, BROADCAST_ADDRESS, NCMP_ADOPTION_ANNOUNCEMENT, 0, 0);
		// wait for the response
		///radio.read(...);
		break;
	}
	return NET_OK;
}


uint8_t net_start(
	network_context_t *context,
	uint8_t channel,
	rnp_address_t address )
{
	uint8_t result;

	if (context == 0) return NETERR_INVALID_INPUT_POINTER;

#if (ENABLE_DIRECT_SEND == 1)
	memset(context->unreachables, 0, sizeof(context->unreachables));
#endif
	// check whether the given address is the broadcast one
	if (address == BROADCAST_ADDRESS)
		return NETERR_INVALID_ADDRESS;
	// setup the internal addresses
	result = net_setAddress(context, address);
	if (result != NET_OK) return result;
	// configure the radio
	nrf24_setChannel(channel);
	nrf24_setPayloadLength(NET_FRAME_LENGTH);
	// configure the radio pipelines and start listening
	nrf24_openReadingPipe(0, BROADCAST_ADDRESS);
	nrf24_openReadingPipe(1, address);
	nrf24_startListening();

	return NET_OK;
}


void net_update(
	network_context_t *context )
{
	rnp_header_t *header;

	while ( nrf24_isDataReady() )
	{
		// fetch the next packet from the radio
		nrf24_receive(context->frameBuffer);
		header = (rnp_header_t*) &context->frameBuffer;

		// check whether we have a valid address
		if ( !net_isValidHostAddress(header->dstAddress) ||
			 !net_isValidHostAddress(header->srcAddress) ) continue;

		printf("Packet received from 0%05o\n", header->srcAddress);

		// check the destination of the packet
		if (header->dstAddress == context->hostAddress)
			net_enqueue(context);
		else
			net_transmit(context);
	}
}


static uint8_t net_enqueue(
	network_context_t *context )
{
	// check whether have a free slot in the queue
	if ( context->nextFrame >= FRAME_QUEUE_ENTRIES )
		return NETERR_QUEUE_FULL;
		
	memcpy( context->frameQueue + context->nextFrame, context->frameBuffer, NET_FRAME_LENGTH );
	context->nextFrame++;
	return NET_OK;
}


static uint8_t net_dequeue(
	network_context_t *context )
{
	// check whether have some entry in the queue
	if ( context->nextFrame == 0 )
		return NETERR_QUEUE_EMPTY;
	
	// retrieve the current frame
	context->nextFrame--;
	memcpy(context->frameBuffer, context->frameQueue + context->nextFrame, NET_FRAME_LENGTH );
	return NET_OK;
}


static uint8_t net_peek(
	network_context_t *context,
	rnp_header_t* header)
{
	if ( context->nextFrame == 0 )
		return NETERR_QUEUE_EMPTY;
	
	// Copy the next available frame from the queue into the provided buffer
	memcpy(&header, context->frameQueue + context->nextFrame, sizeof(rnp_header_t));
	return NET_OK;
}


uint8_t net_receive(
	network_context_t *context,
	rnp_header_t *header,
	uint8_t *data,
	uint8_t *length )
{
	if (data == 0 || header == 0 || length == 0 || *length == 0)
		return NETERR_INVALID_INPUT_POINTER;

	if ( context->nextFrame == 0 )
		return NETERR_QUEUE_EMPTY;

	// move the current frame pointer back one in the queue 
	context->nextFrame -= NET_FRAME_LENGTH;
	// calculate how much data can be copied
	if ( *length > NET_FRAME_LENGTH - sizeof(rnp_header_t) )
		*length = NET_FRAME_LENGTH - sizeof(rnp_header_t);
	// copy the current frame
	memcpy(header, context->frameQueue, sizeof(rnp_header_t));
	memcpy(data, context->frameQueue + sizeof(rnp_header_t), *length);

	return NET_OK;
}


uint8_t net_send(
	network_context_t *context,
	const rnp_header_t *header,
	const uint8_t *data )
{
	rnp_header_t *h = 0;

	if ( context == 0)
		return NETERR_INVALID_CONTEXT;
	if ( header == 0 || data == 0 )
		return NETERR_INVALID_INPUT_POINTER;
	if ( header->dataLength  > NET_FRAME_LENGTH - sizeof(rnp_header_t) )
		return NETERR_PACKET_TOO_LONG;

	// build the frame (header + data)
	memset(context->frameBuffer, 0, NET_FRAME_LENGTH);
	memcpy( context->frameBuffer, header, sizeof(rnp_header_t) );
	if (header->dataLength > 0)
		memcpy(context->frameBuffer + sizeof(rnp_header_t), data, header->dataLength);
	// fill the header with the host information
	h = (rnp_header_t*) context->frameBuffer;
	h->packetId       = context->rnpNextPacketId++;
	h->packetNumber   = 0; // we don't support fragmentation yet
	//h->packetOptions  = context->rnpPacketOptions;
	h->srcAddress     = context->hostAddress;
	if (h->packetHopCount == 0)
		h->packetHopCount = context->rnpPacketMaxHop;
	// if trying to send to himself, simply put the packet in the queue
	if ( header->dstAddress == context->hostAddress )
		return net_enqueue(context);
	else
		// otherwise send it out over the air
		return net_transmit(context);
}


static uint8_t net_transmit(
	network_context_t *context )
{
	uint8_t result;
	const rnp_header_t *header = (const rnp_header_t*)context->frameBuffer;

	// validate the destination address
	if ( !net_isValidHostAddress(header->dstAddress) )
		return NETERR_INVALID_ADDRESS;

	// try to send directly to the destination
#if (ENABLE_DIRECT_SEND == 1)
	if ( header->dstAddress == BROADCAST_ADDRESS ||
	     !net_isUnreachable(context, header->dstAddress) )
#else
	if ( header->dstAddress == BROADCAST_ADDRESS )
#endif
	{
		printf("Trying to send directly\n");
		result = net_write(context, header->dstAddress);
		if (result == NET_OK || header->dstAddress == BROADCAST_ADDRESS )
			return result;
#if (ENABLE_DIRECT_SEND == 1)
		// add the destination to unreachables list
		net_addUnreachable(context, header->dstAddress);
#endif
	}

	printf("OK, I will send through the tree\n");
	// discover what is the next hop and transmit the packet for it
	uint16_t nextHop;
	net_getNextHop(context, header->dstAddress, &nextHop);
	// TODO: detect if the parent is offline
	return net_write(context, nextHop);
}


static uint8_t net_write(
	network_context_t *context,
	rnp_address_t address )
{
	uint8_t result;
	rf24_address_t physical;

	result = net_getRadioAddress(address, &physical);
	if (result != NET_OK) return result;

	// stop listening incoming packets
	nrf24_stopListening();
	// start the transmission
	if ( nrf24_startTransmission() == NRF24_OK )
	{
		result = nrf24_transmit(physical, context->frameBuffer, NET_FRAME_LENGTH);
		if (result == NRF24_OK) nrf24_waitTransmission();
		nrf24_stopTransmission();
	}
	// back to listening mode
	nrf24_startListening();

	return NET_OK;
}


uint8_t net_getNextHop(
	network_context_t *context,
	rnp_address_t destination,
	rnp_address_t *nextHop )
{
	if (nextHop == 0) return NETERR_INVALID_OUTPUT_POINTER;
	// if the destination is broadcast, use broadcast physical address
	if (destination == BROADCAST_ADDRESS)
		*nextHop = BROADCAST_ADDRESS;
	else
	// if the destination is the current node, use the current node address
	if (destination == context->hostAddress)
		*nextHop = context->hostAddress;
	else
	{
		// check whether the destination is a descendant node
		if (net_isChild(context, destination))
		{
			// discover what child node must receive the packet
#if (ENABLE_REDUCE_MEMORY_FOOTPRINT == 1)
			uint16_t mask = NET_HMASK(context->hostAddress) << 3 | 0x07;
#else
			uint16_t mask = (context->hostAddressMask << 3 | 0x07);
#endif
			*nextHop = destination & mask;
		}
		else
			// use the parent as next hop
#if (ENABLE_REDUCE_MEMORY_FOOTPRINT == 1)
			*nextHop = context->hostAddress & (NET_HMASK(context->hostAddress) >> 3);
#else
			*nextHop = context->hostAddress & (context->hostAddressMask) >> 3;
#endif
	}

	return NET_OK;
}

#if (ENABLE_DIRECT_SEND == 1)
uint8_t net_addUnreachable(
	network_context_t *context,
	uint16_t address )
{
	int slot = -1, c = 0;
	
	// find some free slot
	for (c = 0; c < sizeof(context->unreachables) && context->unreachables[c] != 0; ++c)
	{
		// check whether already added
		if ( (context->unreachables[c] & 0x7FFF) == address )
		{
			context->unreachables[c] |= 0x8000;
			return NET_OK;
		}
		// check whether the current entry is an alternative for a free slot
		if ( (context->unreachables[c] & 0x8000) == 0 ) slot = c;
	}
	// if we have a free slot, use it
	if (c < 5)
		slot = c;
	else
	// if we have not some unreachable not use until now, use any
	if (slot < 0)
		slot = 0;
printf("Adding 0%05o to unreachables[%d]\n", address, slot);
	context->unreachables[slot] = address & 0x7FFF;

	return NET_OK;
}


bool net_isUnreachable(
	network_context_t *context,
	rnp_address_t address )
{
	int c;
	for (c = 0; c < sizeof(context->unreachables) && context->unreachables[c] != 0; ++c)
		if (address == (context->unreachables[c] & 0x7FFF))
		{
			// mark the unreachable address as requested
			context->unreachables[c] |= (1 << 15);
			return true;
		}
	return false;
}


static bool ping( uint16_t address )
{
	return rand() % 2 == 0;
}


uint8_t net_updateUnreachables(
	network_context_t *context )
{
	int i = -1;
	int c;
	for (c = 0; c < sizeof(context->unreachables) && context->unreachables[c] != 0; ++c)
	{
		// Note: if the most significant bit is set, the address was used since the
		// last update. In this case, it's better we check if that node is accessible now.
		if (context->unreachables[c] & (1 << 15))
		{
			// ping the node to check whether is accessible
			if ( !ping(context->unreachables[c] & 0x7FFF) )
			{
				// if the node is not accessible yet, don't remove it from the table
				if (i >= 0)
				{
					context->unreachables[i] = context->unreachables[c] & 0x7FFF;
					context->unreachables[c] = 0;
					i++;
				}
				//printf("Keeping (i = %d)\n", i);
				continue;
			}
		}
		// remove the node from the table
		context->unreachables[c] = 0;
		if (i < 0) i = c;
	}
	return NET_OK;
}
#endif // (ENABLE_DIRECT_SEND == 1)


bool net_isDirectChild(
	network_context_t *context,
	rnp_address_t address )
{
	// A direct child of ours has the same low numbers as us, and only
	// one higher number.
	//
	// e.g. node 0234 is a direct child of 034, and node 01234 is a
	// descendant but not a direct child

	// First, is it even a descendant?
	if ( !net_isChild(context, address) ) return false;
	
	// Does it only have ONE more level than us?
#if (ENABLE_REDUCE_MEMORY_FOOTPRINT == 1)
	uint16_t child_node_mask = ( ~ NET_HMASK(context->hostAddress) ) << 3;
#else
	uint16_t child_node_mask = ( ~ context->hostAddressMask ) << 3;
#endif
	return ( context->hostAddress & child_node_mask ) == 0 ;
}


bool net_isChild(
	network_context_t *context,
	rnp_address_t address )
{
#if (ENABLE_REDUCE_MEMORY_FOOTPRINT == 1)
	return ( address & NET_HMASK(context->hostAddress) ) == context->hostAddress;
#else
	return ( address & context->hostAddressMask ) == context->hostAddress;
#endif
}


uint8_t net_setAddress(
	network_context_t *context,
	rnp_address_t address )
{
	// check whether given the broadcast address
	if ( !net_isValidHostAddress(address) )
		return NETERR_INVALID_ADDRESS;
	context->hostAddress = address;
#if (ENABLE_REDUCE_MEMORY_FOOTPRINT == 0)
	context->hostAddressMask = NET_HMASK(context->hostAddress);
#endif
	// generate the physical address
	//result = net_getRadioAddress(address, &context->physNodeAddress);
	//if (result != NET_OK) return result;
		/*for (i = 0; i < 5; ++i)
			if ((context.nodeAddress >> (i * 3)) == 0) break;
		context.nodeAddressMask = 0x7FFF;
		context.physNodeAddressMask = 0x000FFFFF;
		context.nodeAddressMask = context.nodeAddressMask >> (15 - i * 3);
		context.physNodeAddressMask = context.physNodeAddressMask >> (20 - i * 4);*/
	// calculate the address mask
	//context->physNodeAddressMask = NET_RMASK(context->physNodeAddress);

	/*printf("     Node Address : 0%05o\n", context->hostAddress);
	printf("Node Address Mask : 0%05o\n", context->hostAddressMask);*/
	/*printf("     Phys Address : 0%08X\n", context.physNodeAddress);
	printf("Phys Address Mask : 0%08X\n", context.physNodeAddressMask);*/

	return NET_OK;
}


bool net_isValidHostAddress(
	rnp_address_t address )
{
	// check whether the last bit was used
	if ((address >> 15) != 0) return false;
	// check whether we have a zero digit in the middle of the address
	bool hasZero = false;
	int i;
	for (i = 0; i < 5; ++i)
	{
		uint8_t value = (address >> (i*3)) & 0x07;
		if (hasZero && value != 0) return false;
		hasZero |= (value == 0);
	}

	return true;
}


bool net_isValidRadioAddress(
	rf24_address_t address )
{
	// check whether the address have the standard prefix
	if ((address >> 20) != 0xE33) return false;

	return true;
}


uint8_t net_getHostAddress(
	rf24_address_t radioAddress,
	rnp_address_t *hostAddress )
{
	if ( !net_isValidRadioAddress(radioAddress) )
		return NETERR_INVALID_ADDRESS;

	if (radioAddress == BROADCAST_PHYS_ADDRESS)
	{
		*hostAddress = BROADCAST_ADDRESS;
		return NET_OK;
	}

	hostAddress = 0;

	uint8_t w;
	uint8_t shift = 0;
	uint8_t pos = 0;
	while (shift < 20)
	{
		w = PHYSICAL_DIGITS[ ( radioAddress >> shift ) & 0x0F ];
		if (w > 0x07) return false;
		//printf("0%05o\n", w  << pos);
		*hostAddress |= w << pos;
		pos += 3;
		shift += 4;
	}

	return NET_OK;
}


uint8_t net_getRadioAddress(
	rnp_address_t hostAddress,
	rf24_address_t *radioAddress )
{
	if ( !net_isValidHostAddress(hostAddress) )
		return NETERR_INVALID_ADDRESS;

	if (hostAddress == BROADCAST_ADDRESS)
	{
		*radioAddress = BROADCAST_PHYS_ADDRESS;
		return NET_OK;
	}
	*radioAddress = EMPTY_PHYS_ADDRESS;

	uint8_t shift = 0;
	uint8_t pos = 0;
	while (shift < 15)
	{
		//printf("0x%08X\n", LOGICAL_DIGITS[ ( address >> shift ) & 0x07 ] << pos);
		*radioAddress |= LOGICAL_DIGITS[ ( hostAddress >> shift ) & 0x07 ] << pos;
		pos += 4;
		shift += 3;
	}

	return NET_OK;
}


uint32_t net_getMillis()
{
	return clock();
}


void dumpBits( uint16_t n )
{
	int b = 0;
	for (b = 16; b >= 0; --b)
	{
		if (n & (1 << b))
			printf("1");
		else
			printf("0");
		if (b % 8 == 0) printf(" ");
	}
	printf("\n");
}


// vim:ai:cin:sts=4 sw=4 ft=cpp

