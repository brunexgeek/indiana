#include "network.h"


int main( int argc, char **argv )
{
	network_context_t context;
	rnp_header_t header;
	uint32_t data = 0x11223344;

	/*net_initialize(&context, NET_LISTEN_MANUAL);
	net_start(&context, 1, 032);

	header.dstAddress = 03;
	header.dataLength = 4;

	//net_send(&context, &header, (uint8_t*)&data);

	net_update(&context);*/

	printf("Serial: %04x\n", nrf24_getMacAddress());

	printf("Achou? %d\n", (uint8_t)table_findFirst(1) );

	return 0;
}
