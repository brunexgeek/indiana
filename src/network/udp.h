/**
 * @file udp.h
 * @brief Define types and functions to handle User Datagram Protocol (UDP) packets.
 */

#ifndef USER_DATAGRAM_PROTOCOL_H
#define USER_DATAGRAM_PROTOCOL_H


/**
 * Header for User Datagram Protocol (UDP).
 */
typedef struct
{

	/**
	 * Length (in bytes) of the data portion of the UDP datagram. The max value
	 * allowed is 32.
	 */
	uint8_t length : 5;

	/**
	 * Application port number.
	 */
	uint8_t port : 3;

} UDPHeader;


#endif  // USER_DATAGRAM_PROTOCOL_H

