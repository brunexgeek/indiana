#include "network.h"



void nrftest()
{
	nrf24_initialize();

	nrf24_openWritingPipe(012);

	nrf24_send(0, 32);

	nrf24_startWriting();

	nrf24_stopWriting();
}


int main( int argc, char **argv )
{
	network_context_t context;
	rnp_header_t header;
	uint32_t data = 0x11223344;

	net_initialize(&context);
	net_start(&context, 1, 032);

	header.dstAddress = 032;
	header.dataLength = 4;

	net_send(&context, &header, (uint8_t*)&data);

	net_update(&context);

	return 0;
}
