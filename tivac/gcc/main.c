#include <math.h>
#include <stdint.h>
#include <stdbool.h>
#include "inc/hw_ints.h"
#include "inc/hw_memmap.h"
#include "driverlib/debug.h"
#include "driverlib/fpu.h"
#include "driverlib/gpio.h"
#include "driverlib/interrupt.h"
#include "driverlib/pin_map.h"
#include "driverlib/rom.h"
#include "driverlib/sysctl.h"
#include "driverlib/uart.h"
#include "driverlib/timer.h"
#include "driverlib/pwm.h"
#include "gpsParser.h"
#include "altimeter.h"
#include "accelerometer.h"
#include "logger.h"


/******************************************************************************************************/
/***************************************FUNCTION PROTOTYPES********************************************/
/******************************************************************************************************/

void Tiva_setup(void);
void timer_setup(void);
void timer_0A_handler(void);
void timer_1A_handler(void);
void UARTIntHandler(void);
void printStream(void);
void morse_tone_setup(void);



/******************************************************************************************************/
/*******************************************VARIABLES**************************************************/
/******************************************************************************************************/

//For GPS sensor
char gps_buffer[BUFFERSIZE];
char gps_dummy_buffer[750] = "$GPRMC,194426.00,A,4523.03374,N,07541.86434,W,0.281,,280315,,,A*61\r\n$GPVTG,,T,,M,0.281,N,0.521,K,A*2E\r\n$GPGGA,194426.00,9523.03374,S,17541.86434,E,1,06,1.71,30000.874,M,-34.2,M,,*67\r\n$GPGSA,A,3,03,16,31,23,29,,,,,,,,3.57,1.71,3.14*0E\r\n$GPGSV,3,1,11,03,31,251,15,08,71,036,23,14,06,137,08,16,56,212,24*7F\r\n$GPGSV,3,2,11,23,43,305,26,29,19,044,11,31,46,075,25,32,18,205,*75\r\n$GPGSV,3,3,11,46,35,211,,48,14,245,,51,29,221,*4B\r\n$GPGLL,4523.03374,N,07541.86434,W,194426.00,A,A*7E\r\n$GPRMC,194427.00,A,4523.03374,N,07541.86446,W,0.188,,280315,,,A*6F\r\n$GPVTG,,T,,M,0.188,N,0.348,K,A*2D\r\n$GPGGA,194427.00,4523.03374,N,07541.86446,W,1,05,1.71,147.6,M,-34.2,M,,*61\r\n$GPGSA,A,3,03,16,31,23,29,,,,,,,,3.57,1.71,3.14*0";
uint32_t index, count;
GPS_data gps_data;


//For Altimeter
uint16_t PROM_C[8];
Altimeter_data alti_data;  
uint32_t init_Pressure;

//For Accelerometer
int16_t acceletometer_data[3];


//**************************************************************
//
// The error routine that is called if the driver library encounters an error.
//
//**************************************************************
#ifdef DEBUG
void __error__(char *pcFilename, uint32_t ui32Line)
{
}
#endif

//************************************************************
//
//
//
//************************************************************
int main(void)
{

  Tiva_setup();
  // setup the logger
  UART_setup_debug();
  UART_setup_wifiBoard();
  
  I2C1_setup();
  timer_setup();
  alti_setup();
  accel_setup();
  
  logger_logString("Setup complete\n");
  
  init_Pressure = alti_init_pres();

  
  while(1);
}


void UART_setup_debug(void)
{
  //
  // Enable the peripherals used by the debug line.
  //
  ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);
  ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);

  //
  // Set GPIO A0 and A1 as UART pins for the debug.
  //
  ROM_GPIOPinConfigure(GPIO_PA0_U0RX);
  ROM_GPIOPinConfigure(GPIO_PA1_U0TX);
  ROM_GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_0 | GPIO_PIN_1);

  //
  // Configure the UART for 115,200, 8-N-1 operation.
  //
  ROM_UARTConfigSetExpClk(UART0_BASE, ROM_SysCtlClockGet(), 115200,
                          (UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE |
                           UART_CONFIG_PAR_NONE));
}


//**************************************************************
//
// The timer0A interrupt handler.
//
//**************************************************************
void timer_0A_handler(void)
{

  
  ROM_TimerIntClear(TIMER0_BASE, TIMER_TIMA_TIMEOUT);
  parse_gps_data();
  alti_Convert();

  readAccelData();
  calcTiltAnglesFromAccelData();

  printStream();
  ROM_UARTCharPutNonBlocking(UART0_BASE, '\n');
  ROM_UARTCharPutNonBlocking(UART0_BASE, '\n');

}


//**************************************************************
//
// The UART interrupt handler.
//
//**************************************************************
void UARTIntHandler(void)
{
    uint32_t ui32Status;
    char c;
    uint32_t counter = 0;
    //
    // Get the interrrupt status.
    //
    ui32Status = ROM_UARTIntStatus(UART1_BASE, true);

    //
    // Clear the asserted interrupts.
    //
    ROM_UARTIntClear(UART1_BASE, ui32Status);

    //
    // Loop while there are characters in the receive FIFO.
    //
    while (ROM_UARTCharsAvail(UART1_BASE))
    {
      counter++;
        //
        // Read the next character from the UART and write it back to the UART.
        //
        c = ROM_UARTCharGetNonBlocking(UART1_BASE);

        if (index++ >= BUFFERSIZE){
          index = 0;
        }

        gps_buffer[index] = c;
    }
}


//************************************************************
//
// Initialization code for the protocols we use.
//
//************************************************************
void Tiva_setup(void)
{
  //
  // Enable lazy stacking for interrupt handlers.  This allows floating-point
  // instructions to be used within interrupt handlers, but at the expense of
  // extra stack usage.
  //
  ROM_FPUEnable();
  ROM_FPULazyStackingEnable();

  //
  // Set the clocking to run directly from the crystal.
  //
  ROM_SysCtlClockSet(SYSCTL_SYSDIV_1 | SYSCTL_USE_OSC | SYSCTL_OSC_MAIN |
                     SYSCTL_XTAL_16MHZ);
  //
  // Enable processor interrupts.
  //
  ROM_IntMasterEnable();
}



void timer_setup(void){
  ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER0);

  //
  // Configure the two 32-bit periodic timers.
  //
  ROM_TimerConfigure(TIMER0_BASE, TIMER_CFG_PERIODIC);
  ROM_TimerLoadSet(TIMER0_BASE, TIMER_A, ROM_SysCtlClockGet());

  //
  // Setup the interrupts for the timer timeouts.
  //
  ROM_IntEnable(INT_TIMER0A);
  ROM_TimerIntEnable(TIMER0_BASE, TIMER_TIMA_TIMEOUT);

  //
  // Enable the timers.
  //
  ROM_TimerEnable(TIMER0_BASE, TIMER_A);




  //Timer1A for Morsecode call sign, ticks every 500ms
   ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER1);

  //
  // Configure the two 32-bit periodic timers.
  //
  ROM_TimerConfigure(TIMER1_BASE, TIMER_CFG_PERIODIC);
  ROM_TimerLoadSet(TIMER1_BASE, TIMER_A, ROM_SysCtlClockGet()>>1);

  //
  // Setup the interrupts for the timer timeouts.
  //
  ROM_IntEnable(INT_TIMER1A);
  ROM_TimerIntEnable(TIMER1_BASE, TIMER_TIMA_TIMEOUT);

  //
  // Enable the timers.
  //
  ROM_TimerEnable(TIMER1_BASE, TIMER_A);


}


