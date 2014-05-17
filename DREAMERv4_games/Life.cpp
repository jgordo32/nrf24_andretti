#include "Life.h"

void GameOfLife::UpdatePhysics(volatile unsigned char field[BOARD_SIZE][BOARD_SIZE])
{
  int i, j;

  if(pendingReset) {
    ResetGame(field,0, 0);
    return;
  }

  if(movementTimer) {
    movementTimer--;
  }
  else {
    movementTimer = IRQ_HZ/2;

    // Shift the history
    for(i= HIST_SIZE-2; i >= 0; i--) {
      memcpy((void*)&(fieldHist[(i+1)*BOARD_SIZE*BOARD_SIZE]), (void*)&(fieldHist[(i)*BOARD_SIZE*BOARD_SIZE]), sizeof(unsigned char) * BOARD_SIZE * BOARD_SIZE);
    }

    // Find the new state, store it at the head of history (same ptr as newField)
    for(i=0; i < BOARD_SIZE; i++) {
      for(j=0; j < BOARD_SIZE; j++) {
        newField[i][j] = checkSquare(field, i, j);
      }
    }

    // Check for loops
    for(i=0; i < HIST_SIZE; i++) {
      for(j=i+1; j < HIST_SIZE; j++) {
        if(memcmp( &fieldHist[i*BOARD_SIZE*BOARD_SIZE], &fieldHist[j*BOARD_SIZE*BOARD_SIZE], sizeof(unsigned char)*BOARD_SIZE*BOARD_SIZE) == 0) {
          ResetGame(field, 0 , 0);
          movementTimer = IRQ_HZ*3;
          return;
        }
      }
    }

    // Write in the new field
    memcpy((void*)&(field[0][0]), (void*)&(newField[0][0]), sizeof(unsigned char) * BOARD_SIZE * BOARD_SIZE);
  }
}

void GameOfLife::ResetGame(volatile unsigned char field[BOARD_SIZE][BOARD_SIZE], unsigned char isInit, __attribute__((unused)) unsigned char whoWon)
{
  int i, j;

  if(isInit) {
    // Line up the pointers
    for(i=0; i < BOARD_SIZE; i++) {
      newField[i] = &fieldHist[i*BOARD_SIZE];
    }
  }

  // Clear the history, but make them different to avoid finding loops early
  memset(fieldHist, 0, sizeof(unsigned char) * BOARD_SIZE * BOARD_SIZE * HIST_SIZE);

  for(i=0; i < HIST_SIZE; i++) {
    fieldHist[i*BOARD_SIZE*BOARD_SIZE] = i;
  }

  // Randomly initialize the field
  for(i=0; i < BOARD_SIZE; i++) {
    for(j=0; j < BOARD_SIZE; j++) {
      field[i][j] = (random(2) > 0 ? 1 : 0);
    }
  }

  // Start the movement timer
  movementTimer = IRQ_HZ*3;

  pendingReset = 0;
}

void GameOfLife::ProcessInput(
  __attribute__((unused)) volatile unsigned char field[BOARD_SIZE][BOARD_SIZE],
  __attribute__((unused)) long p1ax,
  __attribute__((unused)) long p1ay,
  __attribute__((unused)) char p1bl,
  __attribute__((unused)) char p1br,
  __attribute__((unused)) char p1bu,
  char p1bd,
  __attribute__((unused)) long p2ax,
  __attribute__((unused)) long p2ay,
  __attribute__((unused)) char p2bl,
  __attribute__((unused)) char p2br,
  __attribute__((unused)) char p2bu,
  char p2bd)
{
  if(p1bd || p2bd) {
    pendingReset = 1;
  }
}

unsigned char GameOfLife::checkSquare(volatile unsigned char field[BOARD_SIZE][BOARD_SIZE], int x, int y)
{
  int i, j, count = 0;
  for(i = x-1; i <= x+1; i++) {
    for(j = y-1; j <= y+1; j++) {
      if(!(i == x && j == y)) {
        count += (field[WRAP(i, BOARD_SIZE)][WRAP(j, BOARD_SIZE)] ? 1 : 0);
      }
    }
  }
  // Live cell
  if(field[x][y]) {
    if( count < 2) {
      return 0;// death by underpopulation
    }
    else if(count < 4) {
      return 1; // still alive!
    }
    else {
      return 0; // death by overpopulation
    }
  }
  // dead cell
  else {
    if(count == 3) {
      return 1; // new life!
    }
    else {
      return 0; // still dead
    }
  }
}


