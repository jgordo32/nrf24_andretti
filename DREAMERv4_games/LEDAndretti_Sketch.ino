#include <SPI.h>
#include "DigitalPin.h"
#include "ArduinoGame.h"
#include "Pong.h"
#include "Lightcycle.h"
#include "Life.h"
#include "TicTacToe.h"
#include "Tetris.h"
#include "ArduinoGame.h"

#define HEARTBEAT_IN_MAIN
//#define TEST_PATTERN
//#define SERIAL_PRINT
//#define IRQ_ENABLED

#define NUM_GAMES 5

#define PONG 0
#define TETRIS 1
#define LIGHTCYCLE 2
#define TICTACTOE 3
#define GAMEOFLIFE 4

unsigned char gameMode;
unsigned char downTimer;

#ifndef IRQ_ENABLED
unsigned long microsTimer;
unsigned long currTime;
#endif

// Function Declarations
void LEDShiftOut(byte IC5, byte IC3, byte IC4, byte IC6);
void UpdateGame(void);

// Variable Declarations
DigitalPin<7> latchPin;
DigitalPin<13> lightPin;

DigitalPin<P1_BL> p1bl;
DigitalPin<P1_BR> p1br;
DigitalPin<P1_BU> p1bu;
DigitalPin<P1_BD> p1bd;
DigitalPin<P2_BL> p2bl;
DigitalPin<P2_BR> p2br;
DigitalPin<P2_BU> p2bu;
DigitalPin<P2_BD> p2bd;

// LED field, currently treated as boolean, maybe brightness one day. Can PWM up to 1/8 without much flicker!
volatile unsigned char field[BOARD_SIZE][BOARD_SIZE];

// Heartbeat variables
unsigned long lastTick;
unsigned int toggle;

// Used for drawing the field
int i, j;
byte IC3, IC4, IC5, IC6;
unsigned long PWMcnt;

#ifdef TEST_PATTERN
int testCnt;
#endif

// Manage the Games
ArduinoGame* currentGame;

Pong pong;
Lightcycle lightcycle;
GameOfLife gameoflife;
TicTacToe tictactoe;
Tetris tetris;

void setup()
{
#ifdef SERIAL_PRINT
  Serial.begin(115200);
#endif

  // Seed the RNG
  randomSeed(analogRead(RANDOM_PIN));

  // Clear Variables
  memset((void*)field, 0, sizeof(field));
  lastTick = 0;
  toggle = 0;
  downTimer = 0;
  PWMcnt = 0;
#ifndef IRQ_ENABLED
  microsTimer = 0;
#endif

  gameMode = PONG;
  currentGame = &pong;
  currentGame->ResetGame(field,1,0);

#ifdef TEST_PATTERN
  testCnt = 0;
#endif

  // Enable pins
  latchPin.mode(OUTPUT);
  lightPin.mode(OUTPUT);

  // Internal pullups
  p1bl.mode(INPUT);
  p1bl.high();
  p1br.mode(INPUT);
  p1br.high();
  p1bu.mode(INPUT);
  p1bu.high();
  p1bd.mode(INPUT);
  p1bd.high();
  p2bl.mode(INPUT);
  p2bl.high();
  p2br.mode(INPUT);
  p2br.high();
  p2bu.mode(INPUT);
  p2bu.high();
  p2bd.mode(INPUT);
  p2bd.high();

  // initialize SPI:
  SPI.begin();
  SPI.setClockDivider(SPI_CLOCK_DIV2); // 8MHz

#ifdef IRQ_ENABLED
  // Set up interrupt
  cli();//stop interrupts
  toggle = 0;

  //set timer1 interrupt at IRQ_HZ Hz
  TCCR1A = 0;// set entire TCCR1A register to 0
  TCCR1B = 0;// same for TCCR1B
  TCNT1 = 0;//initialize counter value to 0
  // set compare match register for 1hz increments
  OCR1A = 15625/IRQ_HZ - 1;// (must be <65536) [16MHz / (1024 * IRQ_HZ) - 1]
  // turn on CTC mode
  TCCR1B |= (1 << WGM12);
  // Set CS10 and CS12 bits for 1024 prescaler
  TCCR1B |= (1 << CS12) | (1 << CS10);
  // enable timer compare interrupt
  TIMSK1 |= (1 << OCIE1A);

  sei();//allow interrupts
#endif
}

void loop()
{

#ifdef HEARTBEAT_IN_MAIN
  if(lastTick + 500 < millis()) {
    lastTick = millis();
    if(toggle == 1) {
      toggle = 0;
      lightPin.low();
    }
    else {
      toggle = 1;
      lightPin.high();
    }
  }
#endif

#ifndef IRQ_ENABLED
  currTime = micros();
  if(microsTimer + 1000000/IRQ_HZ < currTime) {
    // Clear the screen for processing
    LEDShiftOut(0x00, 0xFF, 0xFF, 0x00);
    microsTimer = currTime;
    UpdateGame();
  }
#endif
  // Writes 32 bits, 256 times, for a total of 8192 bits. @8MHz SPI, thats 1.024ms.
  // But we have to toggle the latch too, so each set of 32 bits takes 14.76 uS, for a total of 3.78ms
  // This interrupt can be called at most 264Hz (312Hz experimentally, but the LEDs slow down)

  // Iterate through all possible LEDs, flash them if they are on
  for (i = 0; i < BOARD_SIZE; i++) { // X axis, controls VCC
    for (j = 0; j < BOARD_SIZE; j++) { // Y axis, controls GND
      if (field[i][j] /*&& (PWMcnt % field[i][j] == 0)*/) {
        // GND, LSB
        IC3 = ((~(0x8000 >> j)) >> 8) & 0xFF;
        IC4 = ((~(0x8000 >> j)) >> 0) & 0xFF;

        // VCC, MSB
        IC5 = ((0x0001 << i) >> 0) & 0xFF;
        IC6 = ((0x0001 << i) >> 8) & 0xFF;

        LEDShiftOut(IC5, IC3, IC4, IC6);
      }
      else {
        LEDShiftOut(0x00, 0xFF, 0xFF, 0x00);
      }
    }
  }
  PWMcnt++;
}

#ifdef IRQ_ENABLED
//timer1 interrupt 60Hz
ISR(TIMER1_COMPA_vect)
{
  cli();// stop interrupts
  LEDShiftOut(0x00, 0xFF, 0xFF, 0x00);

  // Heartbeat
#ifndef HEARTBEAT_IN_MAIN
  toggle++;
  if (toggle == IRQ_HZ/2) {
    lightPin.high();
  }
  else if(toggle == IRQ_HZ) {
    lightPin.low();
    toggle = 0;
  }
#endif

  UpdateGame();

  sei();// enable interrupts
}
#endif

// First byte controls VCC towards jack
// Second byte controls GND towards jack
// Third byte controls GND away from jack
// Last byte controls VCC away from jack
void LEDShiftOut(byte IC5, byte IC3, byte IC4, byte IC6)
{
  // take the SS pin low to select the chip:
  latchPin.low(); // latch is held low for 14.76 uS
  // send in the address and value via SPI:
  SPI.transfer(IC5);
  SPI.transfer(IC3);
  SPI.transfer(IC4);
  SPI.transfer(IC6);
  // take the SS pin high to de-select the chip:
  latchPin.high();
}

void UpdateGame(void)
{

#ifdef TEST_PATTERN
  field[testCnt%BOARD_SIZE][testCnt/BOARD_SIZE] = 0;
  testCnt = (testCnt+1)%(BOARD_SIZE*BOARD_SIZE);
  field[testCnt%BOARD_SIZE][testCnt/BOARD_SIZE] = 0xFF;
#else

  if(!p1bu.read() && !p2bu.read()) {
    downTimer++;
  }
  else {
    downTimer = 0;
  }

  if(downTimer == IRQ_HZ * 2) {
    downTimer = 0;
    gameMode = (gameMode+1)%NUM_GAMES;
    switch(gameMode) {
      case PONG:
        currentGame = &pong;
        break;
      case LIGHTCYCLE:
        currentGame = &lightcycle;
        break;
      case GAMEOFLIFE:
        currentGame = &gameoflife;
        break;
      case TICTACTOE:
        currentGame = &tictactoe;
        break;
      case TETRIS:
        currentGame = &tetris;
        break;
    }
    currentGame->ResetGame(field,1,0);
  }

  currentGame->ProcessInput(field,
                            analogRead(P1_X), analogRead(P1_Y),
                            !p1bl.read(), !p1br.read(), !p1bu.read(), !p1bd.read(),
                            analogRead(P2_X), analogRead(P2_Y),
                            !p2bl.read(), !p2br.read(), !p2bu.read(), !p2bd.read());
  currentGame->UpdatePhysics(field);
#endif
}
