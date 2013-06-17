#include "network.h"



int main( int argc, char **argv )
{
	network_context_t context;

	net_initialize(&context);
	net_start(&context, 1, 32);

	return 0;
}
