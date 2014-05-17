/*
 * Pong.h
 *
 *  Created on: Oct 27, 2013
 *      Author: adam
 */

#ifndef PONG_H_
#define PONG_H_

#include "ArduinoGame.h"

/*
 * Defines
 */
#define S_M			IRQ_HZ /* Position Multiplier */
#define V_M			IRQ_HZ /* Velocity Multiplier */
#define PADDLE_SIZE	(4 * V_M * S_M) /* in sim size */
#define TIME_STEP	(S_M/IRQ_HZ) /* (1/32) seconds * 32 multiplier */
#define EXTRA_ROTATION_ON_EDGE	15 /*degrees  */
#define SPEED_LIMIT 1200

/*
 * Class
 */
class Pong : public ArduinoGame
{
public:
  Pong() {};
  ~Pong() {};
  void UpdatePhysics(volatile unsigned char field[BOARD_SIZE][BOARD_SIZE]);
  void ResetGame(volatile unsigned char field[BOARD_SIZE][BOARD_SIZE], unsigned char isInit, unsigned char whoWon);
  void ProcessInput(volatile unsigned char field[BOARD_SIZE][BOARD_SIZE], long p1ax, long p1ay, char p1b0, char p1b1, char p1b2, char p1b3, long p2ax, long p2ay, char p2b0, char p2b1, char p2b2, char p2b3);
private:
  void DrawField(volatile unsigned char[BOARD_SIZE][BOARD_SIZE]);
  void IncreaseSpeed(short int speedM);
  void RotateBall(short int degree);
  signed short int binPaddleLocation(long loc);
  signed short int paddleLocL;
  signed short int paddleLocR;
  unsigned short int restartTimer;
  signed short int ballLoc[2];
  signed long int ballVel[2];
};

#endif /* PONG_H_ */




