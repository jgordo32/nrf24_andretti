#ifndef _LIGHTCYCLE_H_
#define _LIGHTCYCLE_H_

#include "ArduinoGame.h"

#define LEFT  0x01
#define RIGHT 0x02
#define UP    0x04
#define DOWN  0x08

class Lightcycle : public ArduinoGame
{
public:
  Lightcycle() {};
  ~Lightcycle() {};
  void UpdatePhysics(volatile unsigned char field[BOARD_SIZE][BOARD_SIZE]);
  void ResetGame(volatile unsigned char field[BOARD_SIZE][BOARD_SIZE], unsigned char isInit, unsigned char whoWon);
  void ProcessInput(volatile unsigned char field[BOARD_SIZE][BOARD_SIZE], long p1ax, long p1ay, char p1b0, char p1b1, char p1b2, char p1b3, long p2ax, long p2ay, char p2b0, char p2b1, char p2b2, char p2b3);
private:
  signed char p1pos[2], p2pos[2];
  unsigned char p1dir, p2dir;
  unsigned char movementTimer;
  unsigned char showingWinningLEDs;
  unsigned char started;
};

#endif
