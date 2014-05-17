#ifndef _GAMEOFLIFE_H_
#define _GAMEOFLIFE_H_

#include "Arduino.h"
#include "ArduinoGame.h"

#define HIST_SIZE 3

class GameOfLife :
  public ArduinoGame
{
public:
  GameOfLife() {
  };
  ~GameOfLife() {
  };
  void UpdatePhysics(volatile unsigned char field[BOARD_SIZE][BOARD_SIZE]);
  void ResetGame(volatile unsigned char field[BOARD_SIZE][BOARD_SIZE], unsigned char isInit, unsigned char whoWon);
  void ProcessInput(volatile unsigned char field[BOARD_SIZE][BOARD_SIZE], long p1ax, long p1ay, char p1b0, char p1b1, char p1b2, char p1b3, long p2ax, long p2ay, char p2b0, char p2b1, char p2b2, char p2b3);
private:
  unsigned char checkSquare(volatile unsigned char field[BOARD_SIZE][BOARD_SIZE], int i, int j);
  unsigned char *newField[BOARD_SIZE]; // Where the new field will be stored during calculation. Convienently at the head of fieldHist
  unsigned char fieldHist[BOARD_SIZE * BOARD_SIZE * HIST_SIZE];
  unsigned char movementTimer;
  unsigned char pendingReset;
};

#endif

