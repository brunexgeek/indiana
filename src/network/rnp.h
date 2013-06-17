/**
 * @file rnp.h
 * @brief Define types and functions for Radio Network Protocol (RNP). This
 * protocol are used to route packets through the network.
 */

#ifndef RADIO_NETWORK_PROTOCOL_H
#define RADIO_NETWORK_PROTOCOL_H


/**
 * Indicate the receiver must send a ACK packet.
 */
#define RNP_OPTION_ACK                     0x01

/**
 * Indicate the current packet is a retransmission of a previous one.
 */
#define RNP_OPTION_RETRANSMIT              0x02


/**
 * Type for logical address of the RNP protocol.
 */
typedef uint16_t rnp_address_t;


/**
 * Header for Radio Network Protocol (RNP) packet.
 */
typedef struct
{

	/**
	 * Sequential ID incremented for every packet sent.
	 */
	uint8_t packetId : 5;

	/**
	 * Number of the fragment. When reassemblying the original packet,
	 * the fragments will be concatenated according this order. The
	 * maximum number of the fragments is 8, giving (8 * MTU) of the "total"
	 * packet length.
	 */
	uint8_t packetNumber : 3;

	/**
	 * @brief Number of hops the datagram can pass through. The default value is 10.
	 *
	 * This value is decreased in one
	 * for every node that relay the datagram. When this value reach zero, the packet
	 * is discarded and the sender is notified with ICMP "Time Exceeded".
	 */
	uint8_t packetHopCount : 4;

	uint8_t packetOptions : 4;

	/**
	 * Protocol used in the data portion of the WNP packet.
	 */
	uint8_t dataProtocol : 3;

	/**
	 * Length (in bytes) of the data portion of the WNP packet. The max value allowed
	 * is 32.
	 */
	uint8_t dataLength : 5;

	/**
	 * Logical address of the message source.
	 */
	rnp_address_t srcAddress;

	/**
	 * Logical address of the message destination.
	 */
	rnp_address_t dstAddress;

} rnp_header_t;


#endif  // WIRELESS_INTERNET_PROTOCOL_H
