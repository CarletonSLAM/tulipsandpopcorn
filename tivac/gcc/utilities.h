
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


void TIVA_wait_miliSeconds(uint32_t ms);
void TIVA_init(void);
char* TIVA_int_to_String(char b[], int i);
int16_t TIVA_find_First_Occurance_Char(char* str, char c);

void TIVA_error_encoutered(const char* strErrorMsg, const int errorNum );


void TIMER0_init(void);
void TIVA_vector_cross_32_16( int32_vec_t *a,  int16_vec_t *b, float_vec_t *out);
void TIVA_vector_cross_16_flaot( int16_vec_t *a,  float_vec_t *b, float_vec_t *out);

float TIVA_vector_dot( int16_vec_t *a,  int16_vec_t *b);

void DEBUGCONSOLE_UART_IntHandler(void);
void DEBUGCONSOLE_add_startDelimeter(void);

char TIVA_checkForConfigMode(void);
void TIVA_configureNetwork(void);

#endif /*__UTILITIES_H_*/
