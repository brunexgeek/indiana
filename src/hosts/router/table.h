#ifndef INDIANA_GATEWAY_H
#define INDIANA_GATEWAY_H


#include <stdio.h>
#include <stdint.h>


typedef struct
{

	/**
	 * Endereço MAC do host que controlará a porta.
	 */
	uint32_t macAddress : 16;

	/**
	 * Número da porta primária a ser controlada.
	 */
	uint32_t portA : 6;

	/**
	 * Número da porta secundária a ser controlada.
	 */
	uint32_t portB : 6;

	uint32_t active : 1;

	uint32_t reserved : 3;

} table_entry_t;


uint8_t table_findFirst(
	uint16_t mac );



#endif  // INDIANA_GATEWAY_H
