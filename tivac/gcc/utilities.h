
#ifndef __UTILITIES_H_
#define __UTILITIES_H_
#include <stdbool.h>
#include <stdint.h>


#define UART_BUFFER_LENGTH 1024

void DEBUGCONSOLE_init(void);

typedef struct int16_vec_t{
  int16_t x;
  int16_t y;
  int16_t z;
}int16_vec_t;


typedef struct int32_vec_t{
  int16_t x;
  int16_t y;
  int16_t z;
}int32_vec_t;

typedef struct float_vctr_t{
  float x;
  float y;
  float z;
}float_vec_t;



void DEBUGCONSOLE_print(const char *pui8Buffer);
void DEBUGCONSOLE_print_line(const char *pui8Buffer);
void DEBUGCONSOLE_print_length(const char *pui8Buffer,uint32_t pui32Count);

int16_t FLASH_write(const uint32_t pui32Address, const char *pui8Data);
int16_t FLASH_read(const uint32_t pui32Address, char* pui8Data);

void TIVA_wait_miliSeconds(uint32_t ms);
void TIVA_init(void);
char* TIVA_int_to_String(char b[], int i);
void TIVA_error_encoutered(const char* strErrorMsg, const int errorNum );


void TIMER0_init(void);
void TIVA_vector_cross_32_16( int32_vec_t *a,  int16_vec_t *b, float_vec_t *out);
void TIVA_vector_cross_16_flaot( int16_vec_t *a,  float_vec_t *b, float_vec_t *out);

float TIVA_vector_dot( int16_vec_t *a,  int16_vec_t *b);

#endif /*__UTILITIES_H_*/
