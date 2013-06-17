#include "network.h"



int main( int argc, char **argv )
{
	network_context_t context;

	net_initialize(&context);
	net_zeroconf(&context, 0);

	return 0;
}
