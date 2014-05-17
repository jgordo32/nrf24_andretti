#include "TicTacToe.h"

void TicTacToe::UpdatePhysics(volatile unsigned char field[BOARD_SIZE][BOARD_SIZE])
{

  unsigned char i, j, full;

  if(cursorTimer[X]) {
    cursorTimer[X]--;
  }
  if(cursorTimer[Y]) {
    cursorTimer[Y]--;
  }

  if(victoryTimer) {
    victoryTimer--;

    if(victoryTimer == 0) {
      ResetGame(field, 0, victory);
    }
    return;
  }

  DrawGrid(field);
  DrawCursor(field);
  DrawPieces(field);

  // Check for victory
  for(i=0; i < 3; i++) {
    if(ttt_board[i][0] != 0 && ttt_board[i][0] == ttt_board[i][1] && ttt_board[i][1] == ttt_board[i][2]) {
      victory = ttt_board[i][0];
      victoryTimer = IRQ_HZ * 3;
      for(j = 0; j < BOARD_SIZE; j++) {
        field[i * 6 + 2][j] = 1;
      }
    }
    if(ttt_board[0][i] != 0 && ttt_board[0][i] == ttt_board[1][i] && ttt_board[1][i] == ttt_board[2][i]) {
      victory = ttt_board[0][i];
      victoryTimer = IRQ_HZ * 3;
      for(j = 0; j < BOARD_SIZE; j++) {
        field[j][i * 6 + 2] = 1;
      }
    }
  }
  if(ttt_board[0][0] != 0 && ttt_board[0][0] == ttt_board[1][1] && ttt_board[1][1] == ttt_board[2][2]) {
    victory = ttt_board[0][0];
    victoryTimer = IRQ_HZ * 3;
    for(j = 0; j < BOARD_SIZE; j++) {
      field[j][j] = 1;
    }
  }
  if(ttt_board[0][2] != 0 && ttt_board[0][2] == ttt_board[1][1] && ttt_board[1][1] == ttt_board[2][0]) {
    victory = ttt_board[0][2];
    victoryTimer = IRQ_HZ * 3;
    for(j = 0; j < BOARD_SIZE; j++) {
      field[j][BOARD_SIZE-j-1] = 1;
    }
  }

  full = 1;
  for(i=0; i < 3; i++) {
    for(j=0; j < 3; j++) {
      if(ttt_board[i][j] == 0) {
        full = 0;
      }
    }
  }
  if(full) {
    victory = '-';
    victoryTimer = IRQ_HZ * 3;
  }
}

void TicTacToe::ResetGame(volatile unsigned char field[BOARD_SIZE][BOARD_SIZE], unsigned char isInit, unsigned char whoWon)
{

  if(isInit) {
    activePlayer = 0;
    cursor[X] = 0;
    cursor[Y] = 0;
    cursorTimer[X] = 0;
    cursorTimer[Y] = 0;
    victory = 0;
    victoryTimer = 0;
  }
  else {
    switch(whoWon) {
      case 'X':
        activePlayer = 1;
        break;
      case 'O':
        activePlayer = 0;
        break;
      case '-':
      default:
        break;
    }
  }
  memset(ttt_board, 0, sizeof(unsigned char) * 3 * 3);

  DrawGrid(field);
  DrawCursor(field);
  DrawPieces(field);
}

void TicTacToe::ProcessInput(
  __attribute__((unused)) volatile unsigned char field[BOARD_SIZE][BOARD_SIZE],
  long p1ax,
  long p1ay,
  __attribute__((unused)) char p1bl,
  __attribute__((unused)) char p1br,
  __attribute__((unused)) char p1bu,
  char p1bd,
  long p2ax,
  long p2ay,
  __attribute__((unused)) char p2bl,
  __attribute__((unused)) char p2br,
  __attribute__((unused)) char p2bu,
  char p2bd)
{
  long analog_x, analog_y;
  char button;

  if(activePlayer == 0) {
    analog_x = p1ax;
    analog_y = p1ay;
    button = p1bd;
  }
  else {
    analog_x = p2ax;
    analog_y = p2ay;
    button = p2bd;
  }

  if(analog_x > 768) {
    if(cursorTimer[X] == 0) {
      cursor[X] = CLAMP(cursor[X]+1, 2);
      cursorTimer[X] = IRQ_HZ/2;
    }
  }
  else if(analog_x < 256) {
    if(cursorTimer[X] == 0) {
      cursor[X] = CLAMP(cursor[X]-1, 2);
      cursorTimer[X] = IRQ_HZ/2;
    }
  }
  else {
    cursorTimer[X] = 0;
  }

  if(analog_y > 768) {
    if(cursorTimer[Y] == 0) {
      cursor[Y] = CLAMP(cursor[Y]-1, 2);
      cursorTimer[Y] = IRQ_HZ/2;
    }
  }
  else if(analog_y < 256) {
    if(cursorTimer[Y] == 0) {
      cursor[Y] = CLAMP(cursor[Y]+1, 2);
      cursorTimer[Y] = IRQ_HZ/2;
    }
  }
  else {
    cursorTimer[Y] = 0;
  }

  if(button && ttt_board[cursor[X]][cursor[Y]] == 0) {
    if(activePlayer == 0) {
      ttt_board[cursor[X]][cursor[Y]] = 'X';
    }
    else {
      ttt_board[cursor[X]][cursor[Y]] = 'O';
    }
    activePlayer = (activePlayer+1)%2;
  }
}

void TicTacToe::DrawGrid(volatile unsigned char field[BOARD_SIZE][BOARD_SIZE])
{
  unsigned char i;

  memset((void*)field, 0, sizeof(unsigned char) * BOARD_SIZE * BOARD_SIZE);

  for(i=0; i < BOARD_SIZE; i++) {
    field[i][4] = 8;
    field[i][5] = 8;
    field[i][10] = 8;
    field[i][11] = 8;
    field[4][i] = 8;
    field[5][i] = 8;
    field[10][i] = 8;
    field[11][i] = 8;
  }
}

void TicTacToe::DrawCursor(volatile unsigned char field[BOARD_SIZE][BOARD_SIZE])
{
  unsigned char i;
  for(i=0; i < 4; i++) {
    field[cursor[X] * 6 + i][cursor[Y] * 6]     = 2;
    field[cursor[X] * 6]    [cursor[Y] * 6 + i] = 2;
  }
}

void TicTacToe::DrawPieces(volatile unsigned char field[BOARD_SIZE][BOARD_SIZE])
{
  unsigned char i, j, x_shift, y_shift;
  for(i=0; i < 3; i++) {
    for(j=0; j < 3; j++) {
      x_shift = i*6;
      y_shift = j*6;
      if(ttt_board[i][j] == 'X') {
        field[1 + x_shift][1 + y_shift] = 1;
        field[1 + x_shift][3 + y_shift] = 1;
        field[2 + x_shift][2 + y_shift] = 1;
        field[3 + x_shift][1 + y_shift] = 1;
        field[3 + x_shift][3 + y_shift] = 1;
      }
      else if(ttt_board[i][j] == 'O') {
        field[1 + x_shift][1 + y_shift] = 1;
        field[1 + x_shift][2 + y_shift] = 1;
        field[1 + x_shift][3 + y_shift] = 1;
        field[2 + x_shift][1 + y_shift] = 1;
        field[2 + x_shift][3 + y_shift] = 1;
        field[3 + x_shift][1 + y_shift] = 1;
        field[3 + x_shift][2 + y_shift] = 1;
        field[3 + x_shift][3 + y_shift] = 1;
      }
    }
  }
}

