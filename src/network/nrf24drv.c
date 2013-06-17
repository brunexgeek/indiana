#include "nrf24.h"


#define PIN_CE                          0x01
#define PIN_CSN                         0x02
#define PIN_SCK                         0x04
#define PIN_MOSI                        0x08
#define PIN_MISO                        0x10

uint8_t pins;

#define CHANGE_BIT(bit,state) \
	{ pins = ((state) != 0) ? pins | (bit) : pins & ~(bit); }


/* -------------------------------------------------------------------------- */
/* In this function you should do the following things:
 *    - Set MISO pin input
 *    - Set MOSI pin output
 *    - Set SCK pin output
 *    - Set CSN pin output
 *    - Set CE pin output     */
/* -------------------------------------------------------------------------- */
void nrf24_setupPins()
{

}

/* -------------------------------------------------------------------------- */
/* nrf24 CE pin control function
 *    - state:1 => Pin HIGH
 *    - state:0 => Pin LOW     */
/* -------------------------------------------------------------------------- */
void nrf24_ce_digitalWrite(uint8_t state)
{
	CHANGE_BIT(PIN_CE, state);
}

/* -------------------------------------------------------------------------- */
/* nrf24 CE pin control function
 *    - state:1 => Pin HIGH
 *    - state:0 => Pin LOW     */
/* -------------------------------------------------------------------------- */
void nrf24_csn_digitalWrite(uint8_t state)
{
	CHANGE_BIT(PIN_CSN, state);
}

/* -------------------------------------------------------------------------- */
/* nrf24 SCK pin control function
 *    - state:1 => Pin HIGH
 *    - state:0 => Pin LOW     */
/* -------------------------------------------------------------------------- */
void nrf24_sck_digitalWrite(uint8_t state)
{
	CHANGE_BIT(PIN_SCK, state);
}

/* -------------------------------------------------------------------------- */
/* nrf24 MOSI pin control function
 *    - state:1 => Pin HIGH
 *    - state:0 => Pin LOW     */
/* -------------------------------------------------------------------------- */
void nrf24_mosi_digitalWrite(uint8_t state)
{
	CHANGE_BIT(PIN_MOSI, state);
}

/* -------------------------------------------------------------------------- */
/* nrf24 MISO pin read function
 * - returns: Non-zero if the pin is high */
/* -------------------------------------------------------------------------- */
uint8_t nrf24_miso_digitalRead()
{
	return pins & PIN_MISO;
}
