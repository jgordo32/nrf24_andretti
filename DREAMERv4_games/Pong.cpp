#include "Arduino.h"
#include "Pong.h"

/**************************
 *                         *
 *         Pong!!          *
 *                         *
 ***************************/

void Pong::UpdatePhysics(volatile unsigned char field[BOARD_SIZE][BOARD_SIZE])
{
  short int diff, rotation;
  if(restartTimer > 0) {
    restartTimer--;
  }
  else {
    // Physics!
    // Update the ball's position
    ballLoc[X] += (ballVel[X] * TIME_STEP);
    ballLoc[Y] += (ballVel[Y] * TIME_STEP);

    // Check for collisions, win conditions. Nudge the ball if necessary
    // Top wall collision
    if (ballLoc[Y] < 0 && ballVel[Y] < 0) {
      ballVel[Y] = -ballVel[Y];
      ballLoc[Y] = 0;
    }
    // Bottom wall collision
    else if (ballLoc[Y] >= BOARD_SIZE * S_M * V_M && ballVel[Y] > 0) {
      ballVel[Y] = -ballVel[Y];
      ballLoc[Y] = (BOARD_SIZE) * S_M * V_M - 1;
    }
    // left paddle collision
    else if (ballLoc[X] < 1 * S_M * V_M && ballVel[X] < 0 && (paddleLocL <= ballLoc[Y] && ballLoc[Y] < paddleLocL + PADDLE_SIZE)) {
      ballVel[X] = -ballVel[X];
      ballLoc[X] = 1 * S_M * V_M; // right on the edge of the paddle

      //Increase speed on collision
      IncreaseSpeed(37); // remember, divided by V_M

      // Apply extra rotation depending on part of the paddle hit
      // range of diff is -PADDLE_SIZE/2 to PADDLE_SIZE/2 (+/- 8192)
      diff = ballLoc[Y] - (paddleLocL + PADDLE_SIZE / 2);
      rotation = (EXTRA_ROTATION_ON_EDGE * diff) / (PADDLE_SIZE / 2); // rotate 45deg at edge of paddle, 0deg in middle, linear in between
      RotateBall(rotation);
    }
    // right paddle collision
    else if (ballLoc[X] >= (BOARD_SIZE - 1) * S_M * V_M && ballVel[X] > 0 && (paddleLocR <= ballLoc[Y] && ballLoc[Y] < paddleLocR + PADDLE_SIZE)) {
      ballVel[X] = -ballVel[X];
      ballLoc[X] = (BOARD_SIZE - 1) * S_M * V_M - 1; // right on the edge of the paddle

      //Increase speed on collision
      IncreaseSpeed(37); // remember, divided by V_M

      // Apply extra rotation depending on part of the paddle hit
      // range of diff is -PADDLE_SIZE/2 to PADDLE_SIZE/2 (+/- 8192)
      diff = ballLoc[Y] - (paddleLocR + PADDLE_SIZE / 2);
      rotation = (EXTRA_ROTATION_ON_EDGE * diff) / (PADDLE_SIZE / 2); // rotate 45deg at edge of paddle, 0deg in middle, linear in between
      RotateBall(-rotation);
    }
    // left wall win
    else if (ballLoc[X] < 0 && ballVel[X] < 0) {
      ResetGame(field, 0, 1);
    }
    // right wall win
    else if (ballLoc[X] >= BOARD_SIZE * S_M * V_M && ballVel[X] > 0) {
      ResetGame(field, 0, 0);
    }
  }
  DrawField(field);
}

void Pong::ResetGame(volatile unsigned char field[BOARD_SIZE][BOARD_SIZE], unsigned char isInit, unsigned char whoWon)
{
  if (isInit) {
    paddleLocL = (V_M * S_M * BOARD_SIZE / 2) - (PADDLE_SIZE / 2);
    paddleLocR = (V_M * S_M * BOARD_SIZE / 2) - (PADDLE_SIZE / 2);
  }
  restartTimer = IRQ_HZ * 3;
  ballLoc[X] = V_M * S_M * (BOARD_SIZE / 2);
  ballLoc[Y] = V_M * S_M * (BOARD_SIZE / 2);
  if(whoWon) {
    ballVel[X] = V_M * 5;
  }
  else {
    ballVel[X] = -V_M * 5;
  }
  ballVel[Y] = (random(7) - 3) * V_M;
  DrawField(field);
}

// Rotate the ball in degrees (-360 -> 359)
void Pong::RotateBall(short int degree)
{
  if (degree < 0) {
    degree += 360;
  }
  if(degree > 359) {
    degree -= 360;
  }

  ballVel[X] = (ballVel[X] * (signed long)cos32[degree] - ballVel[Y] * (signed long)sin32[degree]) / 32;
  ballVel[Y] = (ballVel[X] * (signed long)sin32[degree] + ballVel[Y] * (signed long)cos32[degree]) / 32;
}

// Apply a multiplier to the velocity. make this additive instead?
void Pong::IncreaseSpeed(short int speedM)
{
  ballVel[X] = (ballVel[X] * speedM) / V_M;
  if(ballVel[X] > SPEED_LIMIT) {
    ballVel[X] = SPEED_LIMIT;
  }
  else if(ballVel[X] < -SPEED_LIMIT) {
    ballVel[X] = -SPEED_LIMIT;
  }

  ballVel[Y] = (ballVel[Y] * speedM) / V_M;
  if(ballVel[Y] > SPEED_LIMIT) {
    ballVel[Y] = SPEED_LIMIT;
  }
  else if(ballVel[Y] < -SPEED_LIMIT) {
    ballVel[Y] = -SPEED_LIMIT;
  }
}

void Pong::DrawField(volatile unsigned char field[BOARD_SIZE][BOARD_SIZE])
{
  short int i, j;
  for (i = 0; i < BOARD_SIZE; i++) {
    for (j = 0; j < BOARD_SIZE; j++) {
      if ((j == 0) && ((paddleLocL / (S_M * V_M)) <= i && i < (paddleLocL / (S_M * V_M)) + PADDLE_SIZE / (V_M * S_M))) {
        field[j][i] = 1;
      }
      else if ((j == (BOARD_SIZE - 1)) && ((paddleLocR / (S_M * V_M)) <= i && i < (paddleLocR / (S_M * V_M)) + PADDLE_SIZE / (V_M * S_M))) {
        field[j][i] = 1;
      }
      else if (j == (ballLoc[X] / (S_M * V_M)) && i == (ballLoc[Y] / (S_M * V_M))) {
        field[j][i] = 1;
      }
      else {
        field[j][i] = 0x00;
      }
    }
  }
}

void Pong::ProcessInput(
__attribute__((unused)) volatile unsigned char field[BOARD_SIZE][BOARD_SIZE],
__attribute__((unused)) long p1ax,
long p1ay,
__attribute__((unused)) char p1bl,
__attribute__((unused)) char p1br,
__attribute__((unused)) char p1bu,
__attribute__((unused)) char p1bd,
__attribute__((unused)) long p2ax,
long p2ay,
__attribute__((unused)) char p2bl,
__attribute__((unused)) char p2br,
__attribute__((unused)) char p2bu,
__attribute__((unused)) char p2bd)
{

  if(p1ay < 79){
    paddleLocL = 12288;
  }
  else if(p1ay < 158){
    paddleLocL = 11264;
  }
  else if(p1ay < 236){
    paddleLocL = 10240;
  }
  else if(p1ay < 315){
    paddleLocL = 9216;
  }
  else if(p1ay < 394){
    paddleLocL = 8192;
  }
  else if(p1ay < 473){
    paddleLocL = 7168;
  }
  else if(p1ay < 551){
    paddleLocL = 6144;
  }
  else if(p1ay < 630){
    paddleLocL = 5120;
  }
  else if(p1ay < 709){
    paddleLocL = 4096;
  }
  else if(p1ay < 788){
    paddleLocL = 3072;
  }
  else if(p1ay < 866){
    paddleLocL = 2048;
  }
  else if(p1ay < 945){
    paddleLocL = 1024;
  }
  else if(p1ay < 1024){
    paddleLocL = 0;
  }

  if(p2ay < 79){
    paddleLocR = 12288;
  }
  else if(p2ay < 158){
    paddleLocR = 11264;
  }
  else if(p2ay < 236){
    paddleLocR = 10240;
  }
  else if(p2ay < 315){
    paddleLocR = 9216;
  }
  else if(p2ay < 394){
    paddleLocR = 8192;
  }
  else if(p2ay < 473){
    paddleLocR = 7168;
  }
  else if(p2ay < 551){
    paddleLocR = 6144;
  }
  else if(p2ay < 630){
    paddleLocR = 5120;
  }
  else if(p2ay < 709){
    paddleLocR = 4096;
  }
  else if(p2ay < 788){
    paddleLocR = 3072;
  }
  else if(p2ay < 866){
    paddleLocR = 2048;
  }
  else if(p2ay < 945){
    paddleLocR = 1024;
  }
  else if(p2ay < 1024){
    paddleLocR = 0;
  }
}




