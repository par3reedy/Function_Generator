//Patrick Reedy and Jake Myers; April 26, 2021
/*This source file contains functions pertaining to the operation of TIMERA within the
MSP432P401R Microprocessor from Texas Instruments. */

#include "msp.h"
#include "TIMERA.h"

void TA0_0_IRQHandler (void){
    TIMER_A0->CCTL[0] &= ~TIMER_A_CCTLN_CCIFG;                 //Reset the flag
    count += 1;
    volts = sin_table[count];
    DAC_write(4095);
}
