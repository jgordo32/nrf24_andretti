////////////////////////////////////////////////////////
////////////////////////////////////////////////////////
////    main.c 
////////////////////////////////////////////////////////
////////////////////////////////////////////////////////

#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdio.h>
#include <avr/io.h>
#include <avr/pgmspace.h>
#include <util/delay.h>

/* Scheduler include files */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"

/* Application specific includes */
#include "nrf24.h"
#include "config_types.h"

/* Local Function Prototypes */
void initPorts(void);
void initTimers(void);
void initRadio(void);

void taskHeartbeat2(void *pvParameters);
void radioTask(void *pvParameters);

static int uart_putchar(char c, FILE * stream);
static FILE debug_out = FDEV_SETUP_STREAM(uart_putchar, NULL, _FDEV_SETUP_WRITE);

/* Queue to hold rf transfer data */
xQueueHandle rfQueue;

//TODO: optimize these
/* nrf24 configuration parameters */
#define NRF24_CHANNEL_MHZ		15
#define NRF24_PAYLOAD_LEN		4

#if(BOARD_CONFIG == ARDUINO)
uint8_t ptxAddr[5] = {0xE7, 0xE7, 0xE7, 0xE7, 0xE7};
uint8_t prxAddr[5] = {0xD1, 0xD1, 0xD1, 0xD1, 0xD1};
#elif(BOARD_CONFIG == AVR)
uint8_t ptxAddr[5] = {0xD1, 0xD1, 0xD1, 0xD1, 0xD1};
uint8_t prxAddr[5] = {0xE7, 0xE7, 0xE7, 0xE7, 0xE7};
#endif

/* Main program loop */
int main(void)
{
    
#if 1
    /* Hardware initialization */
    initPorts();
    
    initTimers();
    initRadio();

    rfQueue = xQueueCreate(64, sizeof(uint8_t));  //64 byte queue for RF transfer data

    xTaskCreate(radioTask, (const signed portCHAR *)"Radio", 218, NULL, 2, NULL);
    xTaskCreate(taskHeartbeat2, (const signed portCHAR *)"Blink2", 48, NULL, 1, NULL);


    vTaskStartScheduler();
#else
    //Make PortB.0 an output
    DDRB |= _BV(0);
    while(1)
    {
        // Toggle PortB.0
        PORTB ^= _BV(0);
        _delay_ms(500);
    }    

    
#endif

    return 0;
}

void initRadio(void){
	nrf24_init();
	nrf24_config(NRF24_CHANNEL_MHZ, NRF24_PAYLOAD_LEN);//channel 15, 16 byte payload
	nrf24_tx_address(ptxAddr);
	nrf24_rx_address(prxAddr);

}

/* Blink the LED on PORTC.0 @ 1 Hz */
void taskHeartbeat2(void *pvParameters)
{
	(void) pvParameters;
	uint8_t odd = 0;
	const portTickType xDelay = 500 / portTICK_RATE_MS;
	while(1){
		if(odd)
			PORTB |= _BV(PORTB0);
		else
			PORTB &= ~_BV(PORTB0);
		odd = !odd;
		vTaskDelay(xDelay);
		printf("Heart-beat!\n");
	}
}

/* Send data using the nRF radio */
void radioTask(void *pvParameters)
{
	(void) pvParameters;
	uint8_t rfData[NRF24_PAYLOAD_LEN];
	uint8_t rcvData[NRF24_PAYLOAD_LEN];
	uint8_t pktCounter = 0;
	uint8_t blinkCounter = 0;
	const portTickType blinkDelay = 250 / portTICK_RATE_MS;
	const portTickType sendDelay = 5000 / portTICK_RATE_MS;

	rfData[0] = 0xB1;
	rfData[1] = 0x3A;
	rfData[2] = 0x23;
	while(1)
	{
		if(nrf24_dataReady())
		{
			nrf24_getData(rcvData);
			// Format message for debugging
			char debugRcvString[3*6] = {0};
			for (uint8_t i=0; i < NRF24_PAYLOAD_LEN; i++)
			{
				sprintf(debugRcvString+(3*i), "%02X ", rcvData[i]);
			}
			printf("Received: %s\n", debugRcvString);
		}


		rfData[3] = pktCounter++;
		nrf24_send(rfData);
		while(nrf24_isSending());
		char debugSendString[3*6] = {0};
		for (uint8_t i = 0; i < NRF24_PAYLOAD_LEN; i++)
		{
			sprintf(debugSendString+(3*i), "%02X ", rfData[i]);
		}
		printf("Sent: %s\n", debugSendString);

		if(nrf24_lastMessageStatus())
		{
			//there was a send problem
		}else{
			//transmission was ok
		}
		for(blinkCounter = 1 + nrf24_retransmissionCount(); blinkCounter > 0; --blinkCounter)
		{
			PORTC |= _BV(PORTC3);
			vTaskDelay(blinkDelay);
			PORTC &= ~_BV(PORTC3);
			vTaskDelay(blinkDelay);

		}

		nrf24_powerUpRx();
		vTaskDelay(sendDelay);

	}
}

/* Initialization Functions */

/* Init Ports */
void initPorts(void){
	// Port B initialization
    // 0 - output 	- LED
    // 1 - output 	- rfen						- nRF CE
    // 2 - output 	- /SS						- nRF CSN
    // 3 - output  	- MOSI						- nRF MOSI
    // 4 - input 	- MISO						- nRF MISO
    // 5 - output 	- SCK and arduino LED		- nRF SCK
    // 6 - output 	- NC
    // 7 - output 	- NC

	// DDRB
    // 0b1110 1111 	= 0xEF
	PORTB = 0x00;
	DDRB = 0xEF;

	/* Initialize the hardware SPI module after setting the /SS to output */
	SPCR |= (_BV(MSTR) | _BV(SPE)); //set to master and enable //enabling SPI messes up arduino LED which is on SCK line
	SPCR &= ~(_BV(CPOL) | _BV(CPHA) | _BV(DORD) | _BV(SPR0) | _BV(SPR1)); //set to mode0 with MSB first, f_clk/4
	SPSR |= SPI2X; //this divides min clk cycles by 2 so now SCK = f_clk/2


	// Port C initialization
    // 0 - output 	- NC
    // 1 - input 	- V potentiometer
    // 2 - input 	- H potentiometer
    // 3 - output 	- NC
    // 4 - output 	- NC
    // 5 - output 	- NC
    // 6 - input 	- pulled up, /RESET
    // 7 - output 	- NC

	// DDRC
	// 0b1011 1001	= 0xB9
	PORTC = 0x00;
	DDRC = 0xB9;

	// Port D initialization
    // 0 - input  	- uart RX
    // 1 - output  	- uart TX
    // 2 - input  	- RF interrupt INT0			- nRF IRQ
    // 3 - input 	- joystick button
    // 4 - input  	- button 1
    // 5 - input  	- button 2
    // 6 - input  	- button 3
    // 7 - input  	- button 4
    // NOTE: all buttons need to use internal pull-ups

	// DDRD
    // 0b0000 0010 	= 0x02
   	PORTD = 0x00;
	DDRD = 0x02;
    PORTD = 0xF8; /* Enable pull-ups on the highest 5 pins for the buttons */

	//TODO: don't hardcode this... make it baudrate generic from build settings maybe?
	/* Initialize the hardware UART */
	/* Set baud */
	uint16_t ubrr = 0;
#if(BOARD_CONFIG == ARDUINO)
	ubrr = ((configCPU_CLOCK_HZ / 16) / 57600) - 1;
#elif(BOARD_CONFIG == AVR)
	ubrr = ((configCPU_CLOCK_HZ / 16) / 4800) - 1;
#endif
	UBRR0H = (uint8_t)(ubrr >> 8);
	UBRR0L = (uint8_t)(ubrr);
	/* Enable the receiver and transmitter */
	UCSR0B = (_BV(RXEN0) | _BV(TXEN0));
	/* Set framing to 8N1 */
	UCSR0C = (_BV(UCSZ00) | _BV(UCSZ01));

	/* Initialize the global stream stdout */
	stdout = &debug_out;
}

/* Init Timers */
void initTimers(void){
	// Timer/Counter 0 initialization
	// Clock source: System Clock
	// Clock value: Timer 0 Stopped
	// Mode: Normal top=FFh
	// OC0 output: Disconnected
	// ASSR=0x00;
	// TCCR0=0x00;
	// TCNT0=0x00;
	// OCR0=0x00;

	// Timer/Counter 1 initialization
	// Mode: Normal top=FFFFh


	// OC1A output: Discon.
	// OC1B output: Discon.
	// OC1C output: Discon.
	TCCR1A=0x00;

	// Clock source: none               = 00
	// Clock source: system clock       = 01
	// Clock source: system clock/64    = 03
	// Clock source: system clock/256   = 04
	// Clock source: system clock/1024  = 05
	// Noise Canceler: Off
	// Input Capture on Falling Edge
	TCCR1B=0x00;

    // set the inital counts to zero
	TCNT1H=0x00;
	TCNT1L=0x00;

    // amount to compare the timer to
	OCR1AH=0x00;
	OCR1AL=0x00;

	OCR1BH=0x00;
	OCR1BL=0x00;


	// Timer/Counter 2 initialization
	// Clock source: T2 pin Rising Edge
	// Mode: Normal top=FFh
	// OC2 output: Disconnected
	TCCR2B=0x07;
	TCNT2=0x00;
	OCR2A=0x00;

}

/* Printing to UART function */
static int uart_putchar(char c, FILE * stream)
{
	if (c == '\n')
		uart_putchar('\r', stream);
	while(!(UCSR0A & _BV(UDRE0)));

    UDR0 = c;
    return 0;
}

