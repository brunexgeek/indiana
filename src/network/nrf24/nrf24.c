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


uint8_t nrf24_initialize()
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
	if (pipe == 0) context.firstPipeAddress = address;
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

	return NRF24_OK;
}


/*uint8_t nrf24_openWritingPipe(
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
}*/


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


uint8_t nrf24_setDataRate(
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
/*uint8_t nrf24_payloadLength()
{
    uint8_t status;
    nrf24_csn_digitalWrite(LOW);
    spi_transfer(R_RX_PL_WID);
    status = spi_transfer(0x00);
    return status;
}*/


uint8_t nrf24_receive(
	uint8_t* data )
{
	if (context.isRxMode == 0) return NRF24ERR_INVALID_MODE;

	// TODO: check whether the RX FIFO is empty

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

	return NRF24_OK;
}


void nrf24_getRetransCount(
	uint8_t *count )
{
	spi_getRegister(OBSERVE_TX, count);
	*count = *count & 0x0F;
}


uint8_t nrf24_transmit(
	uint32_t address,
	uint8_t* packet,
	uint16_t packetLen )
{
	if (context.isTxMode == 0) return NRF24ERR_INVALID_MODE;

	// check the parameters
	if (packet == 0) return NRF24ERR_INVALID_INPUT_POINTER;
	if (packetLen != context.payloadLength) return NRF24ERR_INVALID_PACKET_LENGTH;

	// change the TX and RX address (pipe 0)
	spi_writeRegister(RX_ADDR_P0, (uint8_t*)&address, sizeof(uint32_t));
	spi_writeRegister(TX_ADDR, (uint8_t*)&address, sizeof(uint32_t));

	// write the packet to the TX FIFO pading with zero (if necessary)
	nrf24_csn_digitalWrite(LOW);
	spi_transfer(W_TX_PAYLOAD);
	spi_transmitSync(packet, packetLen);
	nrf24_csn_digitalWrite(HIGH);

	// wait for 10us to ensure the at least one packet will be sent if
	// the 'stopTransmission' function is called immidiately
	usleep(10);

	return NRF24_OK;
}


bool nrf24_isTransmitting()
{
	uint8_t status;

	if (context.isTxMode == 0) return FALSE;

	// read the current status
	nrf24_csn_digitalWrite(LOW);
	status = spi_transfer(NOP);
	nrf24_csn_digitalWrite(HIGH);

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


/*uint8_t nrf24_lastMessageStatus()
{
    uint8_t rv;

    rv = nrf24_getStatus(context);

    // Transmission went OK
    if((rv & ((1 << TX_DS))))
    {
       return NRF24_TRANSMISSON_OK;
    }
    // Maximum retransmission count is reached
    // Last message probably went missing ...
    else if((rv & ((1 << MAX_RT))))
    {
        return NRF24_MESSAGE_LOST;
    }  
    // Probably still sending ...
    else
    {
        return 0xFF;
    }
}*/


uint8_t nrf24_startListening()
{
	if (context.isTxMode != 0) return NRF24ERR_INVALID_MODE;
	if (context.isRxMode != 0) return NRF24_OK;

	// flush the RX queue
	nrf24_csn_digitalWrite(LOW);
	spi_transfer(FLUSH_RX);
	nrf24_csn_digitalWrite(HIGH);
	// clear the RX_DR, TX_DS and MAX_RT flags
	spi_setRegister(STATUS,(1<<RX_DR)|(1<<TX_DS)|(1<<MAX_RT)); 
	// power up the radio in RX mode
	nrf24_ce_digitalWrite(LOW);
	spi_setRegister(CONFIG, (1 << PWR_UP) | (1 << PRIM_RX) );
	nrf24_ce_digitalWrite(HIGH);

	return NRF24_OK;
}


uint8_t nrf24_stopListening()
{
	if (context.isRxMode == 0) return NRF24ERR_INVALID_MODE;

	// put the radio in standby-I
	nrf24_ce_digitalWrite(LOW);

	return NRF24_OK;
}


uint8_t nrf24_startTransmission()
{
	if (context.isRxMode != 0) return NRF24ERR_INVALID_MODE;
	if (context.isTxMode != 0) return NRF24_OK;

	// flush the RX queue
	nrf24_csn_digitalWrite(LOW);
	spi_transfer(FLUSH_RX);
	nrf24_csn_digitalWrite(HIGH);
	// clear the RX_DR, TX_DS and MAX_RT flags
	spi_setRegister(STATUS,(1<<RX_DR)|(1<<TX_DS)|(1<<MAX_RT));
	// power up the radio in TX mode (or standby-II if TX FIFO is empty)
	nrf24_ce_digitalWrite(LOW);
	spi_setRegister(CONFIG, (1 << PWR_UP) | (0 << PRIM_RX) );
	nrf24_ce_digitalWrite(HIGH);

	context.isTxMode = TRUE;

	return NRF24_OK;
}


uint8_t nrf24_stopTransmission()
{
	if (context.isTxMode == 0) return NRF24ERR_INVALID_MODE;

	// change back the RX address for pipe 0
	spi_writeRegister(RX_ADDR_P0, (uint8_t*)&context.firstPipeAddress, sizeof(uint32_t));
	// put the radio in standby-I
	nrf24_ce_digitalWrite(LOW);

	context.isTxMode = FALSE;

	return NRF24_OK;
}


uint8_t nrf24_waitTransmission()
{
	uint8_t status, result = 0xFF;
	uint32_t sentAt = 0;

	if (context.isTxMode == 0) return NRF24ERR_INVALID_MODE;

	// This function will block until get TX_DS (transmission completed and ack'd)
	// or MAX_RT (maximum retries, transmission failed). Additionaly, have a timeout
	// (60ms) in case the radio don't set any flag.
	sentAt = clock();
	do
	{
		// read the current status
		nrf24_csn_digitalWrite(LOW);
		status = spi_transfer(NOP);
		nrf24_csn_digitalWrite(HIGH);
		// check if sending successful (TX_DS)
		if ( status & (1 << TX_DS) )
			result = NRF24_OK;
		else
		// check if max retries exceded (MAX_RT)
		if ( status & (1 << MAX_RT) )
			result = NRF24ERR_MAX_RETRANSMISSIONS;
		else
		// check if the timeout is reached
		if ( clock() - sentAt < 60000)
			result = NRF24ERR_TIMEOUT;
	} while(result != 0xFF);

	// clear the RX_DR, TX_DS and MAX_RT flags
	spi_setRegister(STATUS, (1 << RX_DR) | (1 << TX_DS) | (1 << MAX_RT));

	return result;
}


void nrf24_standby()
{
	nrf24_ce_digitalWrite(LOW);
	spi_setRegister(CONFIG, spi_getRegister(CONFIG, 0) | (1 << PWR_UP) );
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


uint8_t spi_transfer(
	uint8_t tx )
{
	//uint8_t i = 0;
	uint8_t rx = 0;    

	nrf24_sck_digitalWrite(LOW);

	/*for(i=0;i<8;i++)
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

	}*/

	nrf24_mosi_digitalWrite(tx & (1 << 7));
	rx |= nrf24_miso_digitalRead() << 7;
	nrf24_mosi_digitalWrite(tx & (1 << 6));
	rx |= nrf24_miso_digitalRead() << 6;
	nrf24_mosi_digitalWrite(tx & (1 << 5));
	rx |= nrf24_miso_digitalRead() << 5;
	nrf24_mosi_digitalWrite(tx & (1 << 4));
	rx |= nrf24_miso_digitalRead() << 4;
	nrf24_mosi_digitalWrite(tx & (1 << 3));
	rx |= nrf24_miso_digitalRead() << 3;
	nrf24_mosi_digitalWrite(tx & (1 << 2));
	rx |= nrf24_miso_digitalRead() << 2;
	nrf24_mosi_digitalWrite(tx & (1 << 1));
	rx |= nrf24_miso_digitalRead() << 1;
	nrf24_mosi_digitalWrite(tx & 1);
	rx |= nrf24_miso_digitalRead();

	return rx;
}


void spi_transferSync(
	uint8_t* dataout,
	uint8_t* datain,
	uint8_t len)
{
	uint8_t i;

	for(i=0;i<len;i++) datain[i] = spi_transfer(dataout[i]);
}


void spi_transmitSync(
	uint8_t* dataout,
	uint8_t len)
{
	uint8_t i;

	for(i=0;i<len;i++) spi_transfer(dataout[i]);
}


void spi_setRegister(
	uint8_t reg,
	uint8_t value)
{
    nrf24_csn_digitalWrite(LOW);
    spi_transfer(W_REGISTER | (REGISTER_MASK & reg));
    spi_transfer(value);
    nrf24_csn_digitalWrite(HIGH);
}


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
