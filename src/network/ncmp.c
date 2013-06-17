#include "ncmp.h"


void ncmp_send(
	network_context_t *context,
	uint16_t destination,
	uint8_t type,
	uint8_t code,
	uint32_t data )
{
	rnp_header_t header;
	ncmp_packet_t ncmp;

	// fill RNP header
	header.dstAddress = destination;
	header.packetOptions = 0;
	// fill NCMP packet
	ncmp.type = type;
	ncmp.code = code;
	ncmp.data = data;
	printf("Sending NCMP packet with type %d\n", type);
	// send the packet
	net_send(context, &header, (uint8_t*)&ncmp);
}
