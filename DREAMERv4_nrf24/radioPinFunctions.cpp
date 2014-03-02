/*
* ----------------------------------------------------------------------------
* THE COFFEEWARE LICENSE (Revision 1):
* <ihsan@kehribar.me> wrote this file. As long as you retain this notice you
* can do whatever you want with this stuff. If we meet some day, and you think
* this stuff is worth it, you can buy me a coffee in return.
* -----------------------------------------------------------------------------
* Please define your platform spesific functions in this file ...
* -----------------------------------------------------------------------------
*/

#include <avr/io.h>
#include "nrf24.h"

#define set_bit(reg,bit) reg |= (1<<bit)
#define clr_bit(reg,bit) reg &= ~(1<<bit)
#define check_bit(reg,bit) (reg&(1<<bit))

/* ------------------------------------------------------------------------- */
void nrf24_setupPins()
{
	/* Setup SPI on the ATmega328P */
	//DDRB |= (_BV(DDB1) | _BV(DDB2) | _BV(DDB3) | _BV(DDB5)); //CE, MOSI, /CSN, SCK
	//DDRB &= ~(_BV(DDB4)); //MISO

//    set_bit(DDRC,0); // CE output
//    set_bit(DDRC,1); // CSN output
//    set_bit(DDRC,2); // SCK output
//    set_bit(DDRC,3); // MOSI output
//    clr_bit(DDRC,4); // MISO input
}
/* ------------------------------------------------------------------------- */
void nrf24_ce_digitalWrite(uint8_t state)
{
    if(state)
    {
        set_bit(PORTB, 4);
    }
    else
    {
        clr_bit(PORTB, 4);
    }
}
/* ------------------------------------------------------------------------- */
void nrf24_csn_digitalWrite(uint8_t state)
{
    if(state)
    {
        set_bit(PORTD, 7);
    }
    else
    {
        clr_bit(PORTD, 7);
    }
}
/* ------------------------------------------------------------------------- */
void nrf24_sck_digitalWrite(uint8_t state)
{
    if(state)
    {
        set_bit(PORTC, 6);
    }
    else
    {
        clr_bit(PORTC, 6);
    }
}
/* ------------------------------------------------------------------------- */
void nrf24_mosi_digitalWrite(uint8_t state)
{
    if(state)
    {
        set_bit(PORTD, 4);
    }
    else
    {
        clr_bit(PORTD, 4);
    }
}
/* ------------------------------------------------------------------------- */
uint8_t nrf24_miso_digitalRead()
{
    return check_bit(PIND, 0);
}
/* ------------------------------------------------------------------------- */
