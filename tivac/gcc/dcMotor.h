#ifndef __DC_MOTOR_H__
#define __DC_MOTOR_H__
#include <stdbool.h>
#include <stdint.h>
#include "utilities.h"


typedef struct {
	uint32_t Base;
  	uint32_t Enable;
 	uint32_t Clockwise;
 	uint32_t CounterClockwise;
} MOTOR_PINS;

void DCMOTOR_init(void);
void DCMOTOR_move_forward_seconds(uint16_t seconds);
void DCMOTOR_move_reverse_seconds(uint16_t seconds);



#endif /*__DC_MOTOR_H__*/