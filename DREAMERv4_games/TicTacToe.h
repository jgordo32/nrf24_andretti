#ifndef _TICTACTOE_H_
#define _TICTACTOE_H_

#include "Arduino.h"
#include "ArduinoGame.h"

class TicTacToe :
  public ArduinoGame
{
public:
  TicTacToe() {
  };
  ~TicTacToe() {
  };
  void UpdatePhysics(volatile unsigned char field[BOARD_SIZE][BOARD_SIZE]);
  void ResetGame(volatile unsigned char field[BOARD_SIZE][BOARD_SIZE], unsigned char isInit, unsigned char whoWon);
  void ProcessInput(volatile unsigned char field[BOARD_SIZE][BOARD_SIZE], long p1ax, long p1ay, char p1b0, char p1b1, char p1b2, char p1b3, long p2ax, long p2ay, char p2b0, char p2b1, char p2b2, char p2b3);
private:
  void DrawCursor(volatile unsigned char field[BOARD_SIZE][BOARD_SIZE]);
  void DrawGrid(volatile unsigned char field[BOARD_SIZE][BOARD_SIZE]);
  void DrawPieces(volatile unsigned char field[BOARD_SIZE][BOARD_SIZE]);
  unsigned char cursor[2];
  unsigned char cursorTimer[2];
  unsigned char activePlayer;
  unsigned char ttt_board[3][3];
  unsigned char victory;
  unsigned char victoryTimer;
};

#endif
