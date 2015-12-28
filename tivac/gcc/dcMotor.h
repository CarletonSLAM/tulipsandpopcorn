#ifndef __DC_MOTOR_H__
#define __DC_MOTOR_H__
#include <stdbool.h>
#include <stdint.h>


typedef struct {
	uint32_t Base;
  	uint32_t Enable;
 	uint32_t Clockwise;
 	uint32_t CounterClockwise;
} MOTOR_PINS;

void DCMOTOR_init(void);
void DCMOTOR_move_forward(void);
void DCMOTOR_stop(void);



#endif /*__DC_MOTOR_H__*/
