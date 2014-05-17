#include "Tetris.h"
#include "Arduino.h"
#include "stdio.h"

Tetris::Tetris()
{
  multiplayer = 1;
}

void Tetris::ResetGame(volatile unsigned char field[BOARD_SIZE][BOARD_SIZE], unsigned char isInit, __attribute__((unused)) unsigned char whoWon)
{
  unsigned char i;
  memset((void*)field, 0, sizeof(unsigned char) * BOARD_SIZE * BOARD_SIZE);

  if (isInit) {
    gameOver = 1;
  }
  else {
    gameOver = 0;
    NewActiveTetromino(field, 1); // ignore return, it will always be placed
    NewActiveTetromino(field, 0); // ignore return, it will always be placed
    DrawActiveTetromino(field);
    DrawNextTetromino(field);
  }
  for(i=0; i < BOARD_SIZE; i++) {
    field[3][i] = 3;
    field[14][i] = 3;
  }

  rotatedYet = 0;
  hardDroppedYet = 0;
  dropTimer = IRQ_HZ/2;
  cursorTimer = 0;
  turboMode = 0;
  rowsCleared = 0;
  leadPlayer = 0;
  downTimer = 0;
}

void Tetris::UpdatePhysics(volatile unsigned char field[BOARD_SIZE][BOARD_SIZE])
{
  if(gameOver) {
    return;
  }

  if(cursorTimer) {
    cursorTimer--;
  }

  if(dropTimer) {
    dropTimer--;
  }
  else {
    DropActiveTetromino(field);

    dropTimer = IRQ_HZ/2 - rowsCleared;
    if(turboMode) {
      dropTimer /= 4;
    }
  }
}

void Tetris::ProcessInput(
  volatile unsigned char field[BOARD_SIZE][BOARD_SIZE],
  long p1ax,
  long p1ay,
  char p1bl,
  char p1br,
  char p1bu,
  char p1bd,
  long p2ax,
  long p2ay,
  char p2bl,
  char p2br,
  char p2bu,
  char p2bd)
{
  long ax, ay;
  char br, bl, bu;

  if(p1bl && p2bl) {
    downTimer++;
  }
  else {
    downTimer = 0;
  }
  if(downTimer == IRQ_HZ * 2) {
    multiplayer = (multiplayer+1)%2;
    ResetGame(field, 1, 0);
    return;
  }

  if(gameOver) {
    if(multiplayer) {
      if(p1bd && p2bd) {
        ResetGame(field, 0, 0);
      }
    }
    else {
      if(p1bd) {
        ResetGame(field, 0, 0);
      }
    }
    return;
  }

  if(multiplayer) {
    if(leadPlayer%2) {
      ax = p1ax;
      ay = p1ay;
      bu = p2bu;
      bl = p2bl;
      br = p2br;
    }
    else {
      ax = p2ax;
      ay = p2ay;
      bu = p1bu;
      bl = p1bl;
      br = p1br;
    }
  }
  else {
    ax = p1ax;
    ay = p1ay;
    bu = p1bu;
    bl = p1bl;
    br = p1br;
  }

  if(bl && !rotatedYet) {
    RotateActiveTetromino(field, CLOCKWISE);
    rotatedYet = 1;
  }
  else if(br && !rotatedYet) {
    RotateActiveTetromino(field, COUNTERCLOCKWISE);
    rotatedYet = 1;
  }
  else if(br == 0 && bl == 0) {
    rotatedYet = 0;
  }

  if(bu && !hardDroppedYet) {
    while(DropActiveTetromino(field)) {
      ;
    }
    hardDroppedYet = 1;
  }
  else if (!bu) {
    hardDroppedYet = 0;
  }

  if(ay < 256 && !turboMode) {
    turboMode = 1;
    dropTimer = 0;
  }
  else if(ay >= 256) {
    turboMode = 0;
  }

  if(ax > 768) {
    if(cursorTimer == 0) {
      SlideActiveTetromino(field, T_RIGHT);
      cursorTimer = IRQ_HZ/8;
    }
  }
  else if(ax < 256) {
    if(cursorTimer == 0) {
      SlideActiveTetromino(field, T_LEFT);
      cursorTimer = IRQ_HZ/8;
    }
  }
  else {
    // reset the timer
    cursorTimer = 0;
  }
}

unsigned char Tetris::NewActiveTetromino(volatile unsigned char field[BOARD_SIZE][BOARD_SIZE], unsigned char isFirst)
{
  unsigned char i;

  if(isFirst) {
    nextType = (tetromino)random(7);
    return 0;
  }
  else {
    activeType = nextType;
    nextType = (tetromino)random(7);
  }

  switch(nextType) {
    case O_TET:
      for(i=0; i < 4; i++) {
        nextTetromino[i][X] = o_tet[0][i][X] - 1;
        nextTetromino[i][Y] = o_tet[0][i][Y] - 1;
      }
      break;
    case I_TET:
      for(i=0; i < 4; i++) {
        nextTetromino[i][X] = i_tet[1][i][X] - 2;
        nextTetromino[i][Y] = i_tet[1][i][Y];
      }
      break;
    case S_TET:
      for(i=0; i < 4; i++) {
        nextTetromino[i][X] = s_tet[1][i][X] - 2;
        nextTetromino[i][Y] = s_tet[1][i][Y];
      }
      break;
    case Z_TET:
      for(i=0; i < 4; i++) {
        nextTetromino[i][X] = z_tet[1][i][X] - 2;
        nextTetromino[i][Y] = z_tet[1][i][Y];
      }
      break;
    case L_TET:
      for(i=0; i < 4; i++) {
        nextTetromino[i][X] = l_tet[1][i][X] - 2;
        nextTetromino[i][Y] = l_tet[1][i][Y];
      }
      break;
    case J_TET:
      for(i=0; i < 4; i++) {
        nextTetromino[i][X] = j_tet[1][i][X] - 2;
        nextTetromino[i][Y] = j_tet[1][i][Y];
      }
      break;
    case T_TET:
      for(i=0; i < 4; i++) {
        nextTetromino[i][X] = t_tet[1][i][X] - 2;
        nextTetromino[i][Y] = t_tet[1][i][Y];
      }
      break;
  }

  switch(activeType) {
    case O_TET:
      for(i=0; i < 4; i++) {
        activeTetromino[i][X] = o_tet[0][i][X];
        activeTetromino[i][Y] = o_tet[0][i][Y];
      }
      break;
    case I_TET:
      for(i=0; i < 4; i++) {
        activeTetromino[i][X] = i_tet[0][i][X];
        activeTetromino[i][Y] = i_tet[0][i][Y];
      }
      break;
    case S_TET:
      for(i=0; i < 4; i++) {
        activeTetromino[i][X] = s_tet[0][i][X];
        activeTetromino[i][Y] = s_tet[0][i][Y];
      }
      break;
    case Z_TET:
      for(i=0; i < 4; i++) {
        activeTetromino[i][X] = z_tet[0][i][X];
        activeTetromino[i][Y] = z_tet[0][i][Y];
      }
      break;
    case L_TET:
      for(i=0; i < 4; i++) {
        activeTetromino[i][X] = l_tet[0][i][X];
        activeTetromino[i][Y] = l_tet[0][i][Y];
      }
      break;
    case J_TET:
      for(i=0; i < 4; i++) {
        activeTetromino[i][X] = j_tet[0][i][X];
        activeTetromino[i][Y] = j_tet[0][i][Y];
      }
      break;
    case T_TET:
      for(i=0; i < 4; i++) {
        activeTetromino[i][X] = t_tet[0][i][X];
        activeTetromino[i][Y] = t_tet[0][i][Y];
      }
      break;
  }

  activeOffset[X] = 7;
  activeOffset[Y] = -2;
  activeRotation = 0;

  for(i=0; i < 4; i++) {
    if(activeTetromino[i][Y] + activeOffset[Y] >= 0 && field[activeTetromino[i][X] + activeOffset[X]][activeTetromino[i][Y] + activeOffset[Y]] != 0) {
      return 1;
    }
  }
  return 0;
}

void Tetris::ClearActiveTetromino(volatile unsigned char field[BOARD_SIZE][BOARD_SIZE])
{
  unsigned char i;
  for(i=0; i < 4; i++) {
    if(activeTetromino[i][Y] + activeOffset[Y] >= 0) {
      field[activeTetromino[i][X] + activeOffset[X]][activeTetromino[i][Y] + activeOffset[Y]] = 0;
    }
  }
}

void Tetris::DrawActiveTetromino(volatile unsigned char field[BOARD_SIZE][BOARD_SIZE])
{
  unsigned char i;
  for(i=0; i < 4; i++) {
    if(activeTetromino[i][Y] + activeOffset[Y] >= 0) {
      field[activeTetromino[i][X] + activeOffset[X]][activeTetromino[i][Y] + activeOffset[Y]] = 1;
    }
  }
}

void Tetris::ClearNextTetromino(volatile unsigned char field[BOARD_SIZE][BOARD_SIZE])
{
  unsigned char i;
  for(i=0; i < 4; i++) {
    field[nextTetromino[i][X]][nextTetromino[i][Y]] = 0;
  }
}

void Tetris::DrawNextTetromino(volatile unsigned char field[BOARD_SIZE][BOARD_SIZE])
{
  unsigned char i;
  for(i=0; i < 4; i++) {
    field[nextTetromino[i][X]][nextTetromino[i][Y]] = 1;
  }
}

/**
 * Returns 1 if the piece dropped or 0 if the piece was blocked
 */
unsigned char Tetris::DropActiveTetromino(volatile unsigned char field[BOARD_SIZE][BOARD_SIZE])
{
  unsigned char i, j, k, rowFull, isBlocked = 0, rc = 0;
  // Check to see if the tetromino can be moved down
  for(i=0; i < 4; i++) {
    if(activeTetromino[i][Y] + activeOffset[Y] + 1 == BOARD_SIZE || // floor
        field[activeTetromino[i][X] + activeOffset[X]][activeTetromino[i][Y] + activeOffset[Y] + 1] == 2) { // set piece
      isBlocked = 1;
    }
  }

  // If the piece is not blocked, move it downward
  if(!isBlocked) {
    ClearActiveTetromino(field);
    activeOffset[Y]++;
    DrawActiveTetromino(field);
    return 1;
  }
  // Otherwise set the piece in the field, clear lines, and try to spawn a new piece
  else {
    leadPlayer = (leadPlayer+1)%2; // whenever a piece is set, swap the controls
    // Set the piece
    for(i=0; i < 4; i++) {
      field[activeTetromino[i][X] + activeOffset[X]][activeTetromino[i][Y] + activeOffset[Y]] = 2;
    }

    // check for rows to clear, clear them
    for(i=0; i < BOARD_SIZE; i++) {
      rowFull = 1;
      for(j=0; j < 10; j++) {
        if(field[j + 3][i] == 0) {
          rowFull = 0;
        }
      }
      if(rowFull) {
        rc++;
        // drop row
        for(j = i; j > 0; j--) {
          for(k=4; k < 14; k++) {
            field[k][j] = field[k][j-1];
          }
        }
        for(k=4; k < 14; k++) {
          field[k][0] = 0;
        }
      }
    }

    // if a row was cleared, add to the score, draw it, and check for victory
    if(rc) {
      rowsCleared += rc;

      for(i=0; i < rowsCleared; i++) {
        if(BOARD_SIZE-1-i >= 0) {
          field[BOARD_SIZE-1][BOARD_SIZE-1-i] = 3;
        }
      }

      if(rowsCleared >= 16) {
        gameOver = 1;
        return 0;
      }
    }

    ClearNextTetromino(field);
    if(NewActiveTetromino(field, 0)) {
      gameOver = 1; // the new tetromino spawned and intersected
    }
    DrawActiveTetromino(field);
    DrawNextTetromino(field);
    return 0;
  }
}

void Tetris::SlideActiveTetromino(volatile unsigned char field[BOARD_SIZE][BOARD_SIZE], signed char direction)
{
  unsigned char i;
  // Check to see if the tetromino can be moved laterally
  for(i=0; i < 4; i++) {
    // 3 is a wall, 2 is a set piece
    if(field[activeTetromino[i][X] + activeOffset[X] + direction][activeTetromino[i][Y] + activeOffset[Y]] == 3 ||
        field[activeTetromino[i][X] + activeOffset[X] + direction][activeTetromino[i][Y] + activeOffset[Y]] == 2) {
      return; // can't slide :(
    }
  }

  // If the piece is not blocked, move it downward
  ClearActiveTetromino(field);
  activeOffset[X] += direction;
  DrawActiveTetromino(field);
}

void Tetris::RotateActiveTetromino(volatile unsigned char field[BOARD_SIZE][BOARD_SIZE], signed char direction)
{

  unsigned char newRotation[4][2];
  unsigned char i;
  unsigned char oldRotation = activeRotation;

  switch(activeType) {
    case O_TET:
      // no rotation
      return;
    case I_TET:
      activeRotation = WRAP(activeRotation + direction, 2);
      for(i=0; i < 4; i++) {
        newRotation[i][X] = i_tet[activeRotation][i][X];
        newRotation[i][Y] = i_tet[activeRotation][i][Y];
      }
      break;
    case S_TET:
      activeRotation = WRAP(activeRotation + direction, 2);
      for(i=0; i < 4; i++) {
        newRotation[i][X] = s_tet[activeRotation][i][X];
        newRotation[i][Y] = s_tet[activeRotation][i][Y];
      }
      break;
    case Z_TET:
      activeRotation = WRAP(activeRotation + direction, 2);
      for(i=0; i < 4; i++) {
        newRotation[i][X] = z_tet[activeRotation][i][X];
        newRotation[i][Y] = z_tet[activeRotation][i][Y];
      }
      break;
    case L_TET:
      activeRotation = WRAP(activeRotation + direction, 4);
      for(i=0; i < 4; i++) {
        newRotation[i][X] = l_tet[activeRotation][i][X];
        newRotation[i][Y] = l_tet[activeRotation][i][Y];
      }
      break;
    case J_TET:
      activeRotation = WRAP(activeRotation + direction, 4);
      for(i=0; i < 4; i++) {
        newRotation[i][X] = j_tet[activeRotation][i][X];
        newRotation[i][Y] = j_tet[activeRotation][i][Y];
      }
      break;
    case T_TET:
      activeRotation = WRAP(activeRotation + direction, 4);
      for(i=0; i < 4; i++) {
        newRotation[i][X] = t_tet[activeRotation][i][X];
        newRotation[i][Y] = t_tet[activeRotation][i][Y];
      }
      break;
  }

  for(i=0; i < 4; i++) {
    if(field[newRotation[i][X] + activeOffset[X]][newRotation[i][Y] + activeOffset[Y]] == 2 || // set piece
        field[newRotation[i][X] + activeOffset[X]][newRotation[i][Y] + activeOffset[Y]] == 3 || // wall
        newRotation[i][Y] + activeOffset[Y] < 0  ||
        newRotation[i][Y] + activeOffset[Y] > 15) {
      activeRotation = oldRotation; // undo
      return; // can't rotate :(
    }
  }

  // rotate the tetromino
  ClearActiveTetromino(field);
  for(i=0; i < 4; i++) {
    activeTetromino[i][X] = newRotation[i][X];
    activeTetromino[i][Y] = newRotation[i][Y];
  }
  DrawActiveTetromino(field);
}


