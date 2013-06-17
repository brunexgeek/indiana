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

#include "nrf24.h"


static nrf24_context_t context;


static void nrf24_config(
	uint8_t channel,
	uint8_t payloadLength )
{
	// use static payload length
	payloadLength = payloadLength & 0x3F;
	// set RF channel
	spi_setRegister(RF_CH, channel);
	// set length of incoming payload 
	spi_setRegister(RX_PW_P0, payloadLength);
	spi_setRegister(RX_PW_P1, payloadLength);
	spi_setRegister(RX_PW_P2, payloadLength);
	spi_setRegister(RX_PW_P3, payloadLength);
	spi_setRegister(RX_PW_P4, payloadLength);
	spi_setRegister(RX_PW_P5, payloadLength);
	// 1 Mbps, TX gain: 0dbm
	spi_setRegister(RF_SETUP, (0 << RF_DR) | (0x03 << RF_PWR));
	// CRC enable with 2 bytes length
	spi_setRegister(CONFIG, (1 << EN_CRC) | (1 << CRCO) | (1 << PRIM_RX));
	// enable auto-acknowledgment for all pipes
	spi_setRegister(EN_AA, 0xFC);
	// disable RX for all pipes
	spi_setRegister(EN_RXADDR, 0);
	// set auto-retransmit delay for 1000 us and up to 3 retransmit trials
	spi_setRegister(SETUP_RETR, (0x04 << ARD) | (0x03 << ARC) );
	// disable dynamic length for all pipes
	spi_setRegister(DYNPD, 0);
}


uint8_t nrf24_initialize(
	nrf24_context_t *context )
{
	nrf24_setupPins();
	nrf24_ce_digitalWrite(LOW);
	nrf24_csn_digitalWrite(HIGH);
	nrf24_config(0, 32);

	return NRF24_OK;
}


uint8_t nrf24_terminate()
{
	return NRF24_OK;
}


uint8_t nrf24_openReadingPipe(
	uint8_t pipe,
	uint32_t address )
{
	uint8_t value;

	if (pipe > 5) return NRF24ERR_INVALID_VALUE;

	// enable the pipe
	spi_getRegister(EN_RXADDR, &value);
	value |= 1 << pipe;
	spi_setRegister(EN_RXADDR,value);
	value = (RX_ADDR_P0 << pipe);
	// we may need to change the pipe 0 address when sending a packet (due
	// auto-acknowledgment), thus must to store the address
	if (pipe == 0)
	{
		context.writePipeAddress = address;
		context.isWritePipeRxMode = TRUE;
	}
	// set the pipe RX address
	if (pipe < 2)
		spi_writeRegister(pipe, (uint8_t*)&address, sizeof(uint32_t));
	else
		spi_writeRegister(pipe, (uint8_t*)&address + 3, 1);

	return NRF24_OK;
}


uint8_t nrf24_closeReadingPipe(
	uint8_t pipe )
{
	uint8_t value;

	// disable the pipe
	spi_getRegister(EN_RXADDR, &value);
	value &= ~(1 << pipe);
	spi_setRegister(EN_RXADDR, value);
	value = (RX_ADDR_P0 << pipe);

	if (pipe == 0) context.isWritePipeRxMode = FALSE;

	return NRF24_OK;
}


uint8_t nrf24_openWritingPipe(
	uint32_t address )
{
	// change the current mode
	context.isTxMode = TRUE;
	// change the RX address for pipe 0
	spi_writeRegister(RX_ADDR_P0, (uint8_t*)&address, sizeof(uint32_t));
	// set the TX address
	spi_writeRegister(TX_ADDR, (uint8_t*)&address, sizeof(uint32_t));

	return NRF24_OK;
}


uint8_t nrf24_closeWritingPipe()
{
	if (!context.isTxMode) return NRF24ERR_INVALID_MODE;
	// reset the pipe 0 configuration
	if (context.isWritePipeRxMode)
		spi_writeRegister(RX_ADDR_P0, (uint8_t*)&context.writePipeAddress, sizeof(uint32_t));
	else
		nrf24_closeReadingPipe(0);
	return NRF24_OK;
}


uint8_t nrf24_setChannel(
	uint8_t channel )
{
	if (channel > 127) return NRF24ERR_INVALID_VALUE;
	spi_setRegister(RF_CH,channel);
	return NRF24_OK;
}


uint8_t nrf24_getChannel(
	uint8_t *channel )
{
	if (channel == 0) return NRF24ERR_INVALID_INPUT_POINTER;
	spi_getRegister(RF_CH, channel);
	return NRF24_OK;
}

/*void nrf24_setAutoAck(
	bool state )
{
	nrf24_setRegister(EN_AA, (state != 0) ? B111111 : 0);
}*/


uint8_t nrf24_setAutoAck(
	uint8_t pipe,
	bool state )
{
	uint8_t value;

	if (pipe > 5 && pipe != 0xFF) return NRF24ERR_INVALID_VALUE;

	if (pipe == 0xFF)
		spi_setRegister(EN_AA, (state != 0) ? 0x3F : 0);
	else
	{
		pipe = (1 << pipe);
		spi_getRegister(EN_AA, &value);
		value = (state != 0) ? value | pipe : value & (~pipe);
		spi_setRegister(EN_AA, value);
	}

	return NRF24_OK;
}


bool nrf24_hasAutoAck(
	uint8_t pipe )
{
	if (pipe > 6) return FALSE;
	pipe = (1 << pipe);
	return spi_getRegister(EN_AA, 0) > 0;
}


uint8_t nrf24_setPowerLevel(
	uint8_t level )
{
	uint8_t value;

	if (level > NRF24_PWR_MAX) return NRF24ERR_INVALID_VALUE;

	spi_readRegister(RF_SETUP, &value, 1);
	value &= ~( 0x03 << RF_PWR );
	value |= level << RF_PWR;
	spi_setRegister(RF_SETUP, value);

	return NRF24_OK;
}


void nrf24_getPowerLevel(
	uint8_t *level )
{
	spi_getRegister(RF_SETUP, level);
	*level = (*level & (0x03 << RF_PWR)) >> RF_PWR;
}


uint8_t nrf24_setDataRatio(
	uint8_t ratio )
{
	uint8_t value;

	if (ratio > NRF24_2MBPS) return NRF24ERR_INVALID_VALUE;

	spi_getRegister(RF_SETUP, &value);
	value &= ~( 0x01 << RF_DR );
	value |= ratio << RF_PWR;
	spi_setRegister(RF_SETUP, value);

	return NRF24_OK;
}


void nrf24_setPayloadLength(
	uint8_t length )
{
	if (length > NRF24_MAX_FRAME_LENGTH)
		length = NRF24_MAX_FRAME_LENGTH;
	context.payloadLength = length;
}


uint8_t nrf24_getPayloadLength(
	uint8_t pipe,
	uint8_t *payloadLength )
{
	if (pipe > 5) return NRF24ERR_INVALID_VALUE;
	if (payloadLength == 0) return NRF24ERR_INVALID_OUTPUT_POINTER;
	// TODO: retrieve the payload for each pipeline
	*payloadLength = context.payloadLength;

	return NRF24_OK;
}


bool nrf24_isDataReady()
{
	// see note in getData() function - just checking RX_DR isn't good enough
	uint8_t status = nrf24_getStatus();
	// we can short circuit on RX_DR...
	if ( status & (1 << RX_DR) ) return TRUE;
	// ...but if it's not set, we still need to check the FIFO for any pending packets
	spi_getRegister(FIFO_STATUS, &status);
	return (status & (1 << RX_EMPTY)) == 0;
}


// TODO: decide what we'll do with this function
uint8_t nrf24_payloadLength()
{
    uint8_t status;
    nrf24_csn_digitalWrite(LOW);
    spi_transfer(R_RX_PL_WID);
    status = spi_transfer(0x00);
    return status;
}


void nrf24_receive(
	uint8_t* data )
{
	// Pull down chip select
	nrf24_csn_digitalWrite(LOW);                               

	// Send cmd to read rx payload
	spi_transfer( R_RX_PAYLOAD );

	// Read payload
	spi_transferSync(data,data,context.payloadLength);

	// Pull up chip select
	nrf24_csn_digitalWrite(HIGH);

	// Reset status register
	spi_setRegister(STATUS,(1<<RX_DR));
}


void nrf24_getRetransCount(
	uint8_t *count )
{
	spi_getRegister(OBSERVE_TX, count);
	*count = *count & 0x0F;
}


void nrf24_send(
	uint8_t* value )
{
	// change to standby-I mode
	nrf24_ce_digitalWrite(LOW);
	// clear the RX_DR, TX_DS and MAX_RT flags
	spi_setRegister(STATUS,(1<<RX_DR)|(1<<TX_DS)|(1<<MAX_RT)); 
	// set to transmitter mode and power up
	spi_setRegister(CONFIG,nrf24_CONFIG|((1<<PWR_UP)|(0<<PRIM_RX)));

	/*
	// Do we really need to flush TX fifo each time ? 
	#if 1
	// Pull down chip select 
	nrf24_csn_digitalWrite(LOW);
	// Write cmd to flush transmit FIFO 
	spi_transfer(FLUSH_TX);
	// Pull up chip select 
	nrf24_csn_digitalWrite(HIGH);
	#endif */

	// write the packet to the TX queue
	nrf24_csn_digitalWrite(LOW);
	spi_transfer(W_TX_PAYLOAD);
	spi_transmitSync(value,context.payloadLength);   
	nrf24_csn_digitalWrite(HIGH);
	// start the transmission (non-blocking)
	nrf24_ce_digitalWrite(HIGH);
}


bool nrf24_isSending()
{
	uint8_t status;

	// read the current status
	status = nrf24_getStatus();

	// if sending successful (TX_DS) or max retries exceded (MAX_RT)
	if((status & ((1 << TX_DS)  | (1 << MAX_RT))))
		return FALSE;

	return TRUE;
}

uint8_t nrf24_getStatus()
{
	uint8_t rv;
	nrf24_csn_digitalWrite(LOW);
	rv = spi_transfer(NOP);
	nrf24_csn_digitalWrite(HIGH);
	return rv;
}

uint8_t nrf24_lastMessageStatus()
{
    uint8_t rv;

    rv = nrf24_getStatus(context);

    /* Transmission went OK */
    if((rv & ((1 << TX_DS))))
    {
       return NRF24_TRANSMISSON_OK;
    }
    /* Maximum retransmission count is reached */
    /* Last message probably went missing ... */
    else if((rv & ((1 << MAX_RT))))
    {
        return NRF24_MESSAGE_LOST;
    }  
    /* Probably still sending ... */
    else
    {
        return 0xFF;
    }
}

void nrf24_startListening()
{
	// flush the RX queue
	nrf24_csn_digitalWrite(LOW);
	spi_transfer(FLUSH_RX);
	nrf24_csn_digitalWrite(HIGH);
	// clear the RX_DR, TX_DS and MAX_RT flags
	spi_setRegister(STATUS,(1<<RX_DR)|(1<<TX_DS)|(1<<MAX_RT)); 
	// power up the radio in RX mode
	nrf24_ce_digitalWrite(LOW);
	spi_setRegister(CONFIG,nrf24_CONFIG|((1<<PWR_UP)|(1<<PRIM_RX)));    
	nrf24_ce_digitalWrite(HIGH);
}


void nrf24_stopListening()
{
	// put the radio in standby-I
	nrf24_ce_digitalWrite(LOW);
}


void nrf24_powerDown()
{
	uint8_t value;

	nrf24_ce_digitalWrite(LOW);
	// put the radio in power down mode
	spi_getRegister(CONFIG, &value);
	value &= ~(1 << PWR_UP);
	spi_setRegister(CONFIG, value);
}


void nrf24_standby()
{
	nrf24_ce_digitalWrite(LOW);
	spi_setRegister(CONFIG, spi_getRegister(CONFIG, 0) | (1 << PWR_UP) );
}


/* software spi routine */
uint8_t spi_transfer(
	uint8_t tx )
{
	uint8_t i = 0;
	uint8_t rx = 0;    

	nrf24_sck_digitalWrite(LOW);

	for(i=0;i<8;i++)
	{

		if(tx & (1<<(7-i)))
			nrf24_mosi_digitalWrite(HIGH);
		else
			nrf24_mosi_digitalWrite(LOW);

		nrf24_sck_digitalWrite(HIGH);

		rx = rx << 1;
		if(nrf24_miso_digitalRead())
			rx |= 0x01;

		nrf24_sck_digitalWrite(LOW);

	}

	return rx;
}

/* send and receive multiple bytes over SPI */
void spi_transferSync(
	uint8_t* dataout,
	uint8_t* datain,
	uint8_t len)
{
	uint8_t i;

	for(i=0;i<len;i++) datain[i] = spi_transfer(dataout[i]);
}

/* send multiple bytes over SPI */
void spi_transmitSync(
	uint8_t* dataout,
	uint8_t len)
{
	uint8_t i;

	for(i=0;i<len;i++) spi_transfer(dataout[i]);
}

/* Clocks only one byte into the given nrf24 register */
void spi_setRegister(
	uint8_t reg,
	uint8_t value)
{
    nrf24_csn_digitalWrite(LOW);
    spi_transfer(W_REGISTER | (REGISTER_MASK & reg));
    spi_transfer(value);
    nrf24_csn_digitalWrite(HIGH);
}

/* Read a single byte from the register of the nrf24 */
uint8_t spi_getRegister(
	uint8_t reg,
	uint8_t *value )
{
	uint8_t temp;
	nrf24_csn_digitalWrite(LOW);
	spi_transfer(R_REGISTER | (REGISTER_MASK & reg));
	spi_transferSync(&temp, &temp, 1);
	nrf24_csn_digitalWrite(HIGH);
	if (value != 0) *value = temp;
	return temp;
}

/* Read single register from nrf24 */
void spi_readRegister(
	uint8_t reg,
	uint8_t* value,
	uint8_t len )
{
    nrf24_csn_digitalWrite(LOW);
    spi_transfer(R_REGISTER | (REGISTER_MASK & reg));
    spi_transferSync(value,value,len);
    nrf24_csn_digitalWrite(HIGH);
}

/* Write to a single register of nrf24 */
void spi_writeRegister(
	uint8_t reg,
	const uint8_t* value,
	uint8_t len )
{
    nrf24_csn_digitalWrite(LOW);
    spi_transfer(W_REGISTER | (REGISTER_MASK & reg));
    spi_transmitSync((uint8_t*)value,len);
    nrf24_csn_digitalWrite(HIGH);
}
