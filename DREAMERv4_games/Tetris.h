#ifndef _TETRIS_H_
#define _TETRIS_H_

#include "ArduinoGame.h"

#define CLOCKWISE 1
#define COUNTERCLOCKWISE -1

#define T_RIGHT 1
#define T_LEFT -1

typedef enum {O_TET, I_TET, S_TET, Z_TET, L_TET, J_TET, T_TET} tetromino;

const unsigned char o_tet[1][4][2] = {
  {{1,1},{1,2},{2,1},{2,2}}, // square
};
const unsigned char i_tet[2][4][2] = {
  {{0,1},{1,1},{2,1},{3,1}},
  {{2,0},{2,1},{2,2},{2,3}}
};
const unsigned char s_tet[2][4][2] = {
  {{1,2},{2,2},{2,1},{3,1}},
  {{2,0},{2,1},{3,1},{3,2}}
};
const unsigned char z_tet[2][4][2] = {
  {{1,1},{2,1},{2,2},{3,2}},
  {{3,0},{3,1},{2,1},{2,2}}
};
const unsigned char l_tet[4][4][2] = {
  {{1,1},{2,1},{3,1},{1,2}},
  {{2,0},{2,1},{2,2},{3,2}},
  {{1,1},{2,1},{3,1},{3,0}},
  {{2,0},{2,1},{2,2},{1,0}},
};
const unsigned char j_tet[4][4][2] = {
  {{1,1},{2,1},{3,1},{3,2}},
  {{2,0},{2,1},{2,2},{3,0}},
  {{1,1},{2,1},{3,1},{1,0}},
  {{2,0},{2,1},{2,2},{1,2}},
};
const unsigned char t_tet[4][4][2] = {
  {{1,1},{2,1},{3,1},{2,2}},
  {{2,0},{2,1},{2,2},{3,1}},
  {{1,1},{2,1},{3,1},{2,0}},
  {{2,0},{2,1},{2,2},{1,1}},
};

class Tetris : public ArduinoGame
{
public:
  Tetris();
  ~Tetris() {};
  void UpdatePhysics(volatile unsigned char field[BOARD_SIZE][BOARD_SIZE]);
  void ResetGame(volatile unsigned char field[BOARD_SIZE][BOARD_SIZE], unsigned char isInit, unsigned char whoWon);
  void ProcessInput(volatile unsigned char field[BOARD_SIZE][BOARD_SIZE], long p1ax, long p1ay, char p1b0, char p1b1, char p1b2, char p1b3, long p2ax, long p2ay, char p2b0, char p2b1, char p2b2, char p2b3);
private:
  // Drawing
  void ClearActiveTetromino(volatile unsigned char field[BOARD_SIZE][BOARD_SIZE]);
  void DrawActiveTetromino(volatile unsigned char field[BOARD_SIZE][BOARD_SIZE]);
  void ClearNextTetromino(volatile unsigned char field[BOARD_SIZE][BOARD_SIZE]);
  void DrawNextTetromino(volatile unsigned char field[BOARD_SIZE][BOARD_SIZE]);
  // Moving
  unsigned char NewActiveTetromino(volatile unsigned char field[BOARD_SIZE][BOARD_SIZE], unsigned char isFirst);
  void RotateActiveTetromino(volatile unsigned char field[BOARD_SIZE][BOARD_SIZE], signed char direction);
  unsigned char DropActiveTetromino(volatile unsigned char field[BOARD_SIZE][BOARD_SIZE]);
  void SlideActiveTetromino(volatile unsigned char field[BOARD_SIZE][BOARD_SIZE], signed char direction);
  // Timers
  unsigned char dropTimer;
  unsigned char rotatedYet;
  unsigned char hardDroppedYet;
  unsigned char cursorTimer;
  unsigned char turboMode;
  // State
  signed char nextTetromino[4][2];
  signed char activeTetromino[4][2];
  signed char activeOffset[2];
  unsigned char activeRotation;
  tetromino activeType;
  tetromino nextType;
  unsigned char gameOver;
  unsigned char rowsCleared;
  unsigned char leadPlayer;
  unsigned char multiplayer;
  unsigned char downTimer;
};

#endif
