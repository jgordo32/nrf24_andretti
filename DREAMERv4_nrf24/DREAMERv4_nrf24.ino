#include "nrf24.h"
#include "nRF24L01.h"

/* nrf24 configuration parameters */
#define NRF24_CHANNEL_MHZ		9
#define NRF24_PAYLOAD_LEN		1

/* The arduino duemi has the opposite address settings */
uint8_t ptxAddr[5] = {0xD3, 0xD3, 0xD3, 0xD3, 0xD3};
uint8_t prxAddr[5] = {0xE8, 0xE8, 0xE8, 0xE8, 0xE8};

/* local function prototypes */
void init_radio(void);
void radio_task(void);

void setup()
{
  /* pin initialization */
  DDRB |= (1 << 4);/* nRF CE     ->  nano D8    -> 32u4 PortB.4 */
  DDRD |= (1 << 7);/* nRF CSN    ->  nano D6    -> 32u4 PortD.7 */
  DDRC |= (1 << 6);/* nRF SCK     ->  nano D5    -> 32u4 PortC.6 */
  DDRD |= (1 << 4);/* nRF MOSI    ->  nano D4  -> 32u4 PortD.4 */
  DDRD &= ~(1 << 0);/* nRF MISO    ->  nano D3  -> 32u4 PortD.0 */
  DDRD &= ~(1 << 1);/* nRF IRQ    ->  nano D2  -> 32u4 PortD.1 */
  
  DDRC |= (1 << 7); /* LED -> nano A0 -> 32u4 PortC.7 */
  
   /* USB CDC Serial Debugging */
  Serial.begin(57600);
  
  delay(8000);
  
  init_radio();
    
  Serial.println("Hello Leonardo! Initialization complete!");
  Serial.print("Current RAM payload length: ");
  Serial.println(nrf24_payload_length());
  
}

void loop()
{
    radio_task();
}

void init_radio(void)
{
    Serial.println("Initializing radio...");
    nrf24_init();
    Serial.println("Configuring channel and payload...");
    nrf24_config(NRF24_CHANNEL_MHZ, NRF24_PAYLOAD_LEN);//channel 15, 16 byte payload
    nrf24_tx_address(ptxAddr);
    nrf24_rx_address(prxAddr);
}

void radio_task(void)
{
    uint8_t rfData[NRF24_PAYLOAD_LEN];
    uint8_t rcvData[NRF24_PAYLOAD_LEN];
    uint8_t pktCounter = 0;
    int8_t blinkCounter = 0;
    
    rfData[0] = 0xF1;
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
	    Serial.print("Received: ");
            Serial.println(debugRcvString);
	}

	rfData[3] = pktCounter++;
	nrf24_send(rfData);
	while(nrf24_isSending());
	char debugSendString[3*6] = {0};
	for (uint8_t i = 0; i < NRF24_PAYLOAD_LEN; i++)
	{
            sprintf(debugSendString+(3*i), "%02X ", rfData[i]);
	}
	Serial.print("Sent: ");
        Serial.println(debugSendString);

	if(nrf24_lastMessageStatus())
	{
	    //there was a send problem
	}else{
	    //transmission was ok
	}
	

        for(blinkCounter = 1 + nrf24_retransmissionCount(); blinkCounter > 0; --blinkCounter)
	{
	    Serial.println("Blinking LED!");
            PORTC |= (1 << 7);
	    delay(500);
	    PORTC &= ~(1 << 7);
            delay(500);
	}

	nrf24_powerUpRx();
	delay(3000);
    }
}
