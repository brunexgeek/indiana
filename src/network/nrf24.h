/*
* ----------------------------------------------------------------------------
* “THE COFFEEWARE LICENSE” (Revision 1):
* <ihsan@kehribar.me> wrote this file. As long as you retain this notice you
* can do whatever you want with this stuff. If we meet some day, and you think
* this stuff is worth it, you can buy me a coffee in return.
* -----------------------------------------------------------------------------
* This library is based on this library: 
*   https://github.com/aaronds/arduino-nrf24l01
* Which is based on this library: 
*   http://www.tinkerer.eu/AVRLib/nRF24L01
* -----------------------------------------------------------------------------
*/
#ifndef NRF24
#define NRF24

#include <stdint.h>
#include <unistd.h>
#include "nrf24l01p.h"


#define LOW 0
#define HIGH 1

/**
 * -18dBm
 */
#define NRF24_PWR_MIN                   0x00

/**
 * -12dBm
 */
#define NRF24_PWR_LOW                   0x01

/**
 * -6dBm
 */ 
#define NRF24_PWR_HIGH                  0x02

/**
 * 0dBm
 */
#define NRF24_PWR_MAX                   0x03

#define NRF24_1MBPS                     0x00
#define NRF24_2MBPS                     0x01


#define NRF24_MAX_FRAME_LENGTH          0x20


#define nrf24_ADDR_LEN 4
#define nrf24_CONFIG ((1<<EN_CRC)|(0<<CRCO))

#define NRF24_TRANSMISSON_OK 0
#define NRF24_MESSAGE_LOST   1


#define NRF24_OK                             ((uint8_t)0x00)
#define NRF24ERR_INVALID_VALUE               ((uint8_t)0x01)
#define NRF24ERR_INVALID_INPUT_POINTER       ((uint8_t)0x02)
#define NRF24ERR_INVALID_OUTPUT_POINTER      ((uint8_t)0x02)
#define NRF24ERR_INVALID_MODE                ((uint8_t)0x03)
#define NRF24ERR_INVALID_PACKET_TOO_LONG     ((uint8_t)0x04)

#ifndef __cplusplus
typedef uint8_t bool;
#define TRUE 1
#define FALSE 0
#endif


typedef struct
{

	/**
	 * Indicates if the network is in TX mode.
	 *
	 * @see openWritingPipe
	 */
	uint8_t isTxMode : 1;

	/**
	 * Indicates if the pipe 0 is open for reading.
	 */
	uint8_t	isWritePipeRxMode : 1;

	uint8_t reserved : 7;

	/**
	 * Default payload length, used in all pipes.
	 */
	uint8_t payloadLength;

	/**
	 * Store the address for the pipe 0, because while writing can be needed
	 * change the RX address due auto-acknowledgement.
	 */
	uint32_t writePipeAddress;

} nrf24_context_t;


/**
 * Initialize the hardware pins and basic configuration.
 */
uint8_t nrf24_initialize();

uint8_t nrf24_terminate();


/**
* Open a pipe for reading
*
* Up to 6 pipes can be open for reading at once. Open all necessary
* reading pipes and then call startListening().
*
* @warning Pipes 1-5 should share the first 32 bits.
* Only the least significant byte should be unique, e.g.
* @code
*   openReadingPipe(1,0xF0F0F0F0AA);
*   openReadingPipe(2,0xF0F0F0F066);
* @endcode
* This way, when using the pipes from 2, ensure to be opened the
* pipe 1 before to call startListening().
*
* @warning Pipe 0 is also used by the writing pipe.  So if you open
* pipe 0 for reading, and then startListening(), it will overwrite the
* writing pipe.  Ergo, do an openWritingPipe() again before write().
*
* @todo Enforce the restriction that pipes 1-5 must share the top 32 bits
*
* @param number Which pipe# to open, 0-5.
* @param address The 40-bit address of the pipe to open.
*/
uint8_t nrf24_openReadingPipe(
	uint8_t pipe,
	uint32_t address );


/**
 * Set the channel number from 0 to 127.
 */
uint8_t nrf24_setChannel(
	uint8_t channel );

/**
 * Enable or disable the auto-acknowledgment for indivial pipes. If
 * the given pipe number is 0xFF, all pipes will be configured.
 */
uint8_t nrf24_setAutoAck(
	uint8_t pipe,
	bool state );

/**
 * Returns if the auto-acknowledgement is enable for the given pipe.
 */
bool nrf24_hasAutoAck(
	uint8_t pipe );

/**
 * Define the power level for the radio transmiter. The valid levels
 * are NRF24_PWR_MIN, NRF24_PWR_LOW, NRF24_PWR_HIGH and NRF24_PWR_MAX.
 */
uint8_t nrf24_setPowerLevel(
	uint8_t level );

/**
 * Returns the configured power level for the radio transmiter.
 */
void nrf24_getPowerLevel(
	uint8_t *level );

/**
 * Define the data ratio used by the transmiter. The valid rations are
 * NRF24_1MBPS and NRF24_2MBPS.
 */
uint8_t nrf24_setDataRatio(
	uint8_t ratio );

/**
 * Define the four-byte radio address for the given pipe.
 */
uint8_t nrf24_setRxAddress(
	uint8_t pipe,
	uint32_t address );


/**
 * Define the payload length for all receivers and the transmiter.
 */
void nrf24_setPayloadLength(
	uint8_t length );

/**
 * Returns the payload length for the given pipeline.
 */
uint8_t nrf24_getPayloadLength(
	uint8_t pipe,
	uint8_t *payloadLength );

/**
 * Define the four-byte radio address for the transmiter.
 */
void nrf24_setTxAddress(
	uint32_t address );

/**
 * Returns a boolean value indicating if some data is
 * available for reading.
 */
bool nrf24_isDataReady();

/**
 * Returns a boolean value indicating if the receiver queue
 * if empty.
 */
bool nrf24_isRxFifoEmpty();

/**
 * Reads payload bytes into data array.
 */
void nrf24_receive(
	uint8_t* data );

/**
 * Returns the number of retransmissions occured for the last message.
 */
void nrf24_getRetransCount(
	uint8_t *count );

/**
 * Put a packet in the TX FIFO. Be sure to send the correct
 * amount of bytes as configured as payload on the receiver.
 */
uint8_t nrf24_send(
	uint8_t* packet,
	uint16_t packetLen );

/**
 * Returns a boolean value indicating if the radio is currently sending a packet.
 */
bool nrf24_isSending();

uint8_t nrf24_getStatus();

uint8_t nrf24_lastMessageStatus();

/**
 * Enter in RX mode.
 */
void nrf24_startListening();

/**
 * Exit the RX mode.
 */
void nrf24_stopListening();

/**
 * Enter in standby-II mode to send packets.
 *
 */
void nrf24_startWriting();

/**
 * Exit standby-II mode.
 */
void nrf24_stopWriting();

// TODO: merge this with 'send'.
uint8_t nrf24_openWritingPipe(
	uint32_t address );

// TODO: merge this with 'stopWriting'
uint8_t nrf24_closeWritingPipe();

void nrf24_standby();

void nrf24_powerDown();

/*
 * Low level functions interface.
 */

uint8_t spi_transfer(
	uint8_t tx );

void spi_transferSync(
	uint8_t* dataout,
	uint8_t* datain,
	uint8_t len);

void spi_transmitSync(
	uint8_t* dataout,
	uint8_t len);

void spi_setRegister(
	uint8_t reg,
	uint8_t value );

uint8_t spi_getRegister(
	uint8_t reg,
	uint8_t *value );

void spi_readRegister(
	uint8_t reg,
	uint8_t* value,
	uint8_t len );

void spi_writeRegister(
	uint8_t reg,
	const uint8_t* value,
	uint8_t len );

/*
// adjustment functions 
void    nrf24_init();
void    nrf24_rx_address(uint8_t* adr);
void    nrf24_tx_address(uint8_t* adr);

uint8_t nrf24_config(
	nrf24_context_t *context,
	uint8_t channel,
	uint8_t payloadLength );

// state check functions 
uint8_t nrf24_dataReady();
uint8_t nrf24_isSending();
uint8_t nrf24_getStatus();
uint8_t nrf24_rxFifoEmpty();

// core TX / RX functions 
void    nrf24_send(uint8_t* value);
void    nrf24_receive(uint8_t* data);

// use in dynamic length mode 
uint8_t nrf24_payloadLength();

// post transmission analysis 
uint8_t nrf24_lastMessageStatus();
uint8_t nrf24_retransmissionCount();

// Returns the payload length 
uint8_t nrf24_payload_length();

// power management 
void    nrf24_powerUpRx();
void    nrf24_powerUpTx();
void    nrf24_powerDown();

// low level interface ... 
uint8_t spi_transfer(uint8_t tx);
void    nrf24_transmitSync(uint8_t* dataout,uint8_t len);
void    nrf24_transferSync(uint8_t* dataout,uint8_t* datain,uint8_t len);
void    nrf24_configRegister(uint8_t reg, uint8_t value);
void    nrf24_readRegister(uint8_t reg, uint8_t* value, uint8_t len);
void    nrf24_writeRegister(uint8_t reg, uint8_t* value, uint8_t len);
*/
/* -------------------------------------------------------------------------- */
/* You should implement the platform spesific functions in your code */
/* -------------------------------------------------------------------------- */

/* -------------------------------------------------------------------------- */
/* In this function you should do the following things:
 *    - Set MISO pin input
 *    - Set MOSI pin output
 *    - Set SCK pin output
 *    - Set CSN pin output
 *    - Set CE pin output     */
/* -------------------------------------------------------------------------- */
extern inline void nrf24_setupPins();

/* -------------------------------------------------------------------------- */
/* nrf24 CE pin control function
 *    - state:1 => Pin HIGH
 *    - state:0 => Pin LOW     */
/* -------------------------------------------------------------------------- */
extern void nrf24_ce_digitalWrite(uint8_t state);

/* -------------------------------------------------------------------------- */
/* nrf24 CE pin control function
 *    - state:1 => Pin HIGH
 *    - state:0 => Pin LOW     */
/* -------------------------------------------------------------------------- */
extern void nrf24_csn_digitalWrite(uint8_t state);

/* -------------------------------------------------------------------------- */
/* nrf24 SCK pin control function
 *    - state:1 => Pin HIGH
 *    - state:0 => Pin LOW     */
/* -------------------------------------------------------------------------- */
extern void nrf24_sck_digitalWrite(uint8_t state);

/* -------------------------------------------------------------------------- */
/* nrf24 MOSI pin control function
 *    - state:1 => Pin HIGH
 *    - state:0 => Pin LOW     */
/* -------------------------------------------------------------------------- */
extern void nrf24_mosi_digitalWrite(uint8_t state);

/* -------------------------------------------------------------------------- */
/* nrf24 MISO pin read function
 * - returns: Non-zero if the pin is high */
/* -------------------------------------------------------------------------- */
extern uint8_t nrf24_miso_digitalRead();

#endif
