/*Patrick Reedy; May 12, 2021
 * This assignment's purpose was to use the MSP432P401R to interface with the
 * MCP4921 Digital to Analog Converter (DAC) to be able to output voltages inputted
 * from a keypad.*/

/* Data Sheets for Parts Used
 * MCU:
 * https://www.ti.com/lit/ug/slau356i/slau356i.pdf?ts=1619924832815&ref_url=https%253A%252F%252Fwww.ti.com%252Fproduct%252FMSP432P401R
 * DAC: https://ww1.microchip.com/downloads/en/DeviceDoc/22248a.pdf
 * Keypad: https://www.parallax.com/product/4x4-matrix-membrane-keypad/
 */


/*--------------------------------------------------------------------------------------------------------------------------------------
*  Library Definitions
--------------------------------------------------------------------------------------------------------------------------------------*/
#include "msp.h"
#include "dac.h"
#include "DCO.h"
#include "KEYPAD.h"
#include "FG.h"
#include "math.h"

/*--------------------------------------------------------------------------------------------------------------------------------------
*  Functions Used from other Source Files
--------------------------------------------------------------------------------------------------------------------------------------*/
void DAC_init(void);
void DAC_write(uint16_t data);
uint16_t DAC_volt_conv(uint16_t keypad_volts);
void set_DCO(uint32_t freq);
void keypad_init(void);
char get_key(void);

/*--------------------------------------------------------------------------------------------------------------------------------------
*  Main
--------------------------------------------------------------------------------------------------------------------------------------*/
volatile uint16_t count = 0;                                            //interrupt count value for wave arrays
uint16_t sin_table[MAXCOUNT],                                           //array for a sine wave
         ramp_table[MAXCOUNT],                                          //array for a ramp wave
         tri_table[MAXCOUNT],                                           //array for a triangle wave
         wave_table[MAXCOUNT];                                          //array for the interrupt wave being plotted
uint8_t frequency, waveform, duty_cycle;                                //variables used to change the waveform and its properties
int8_t BTNPRESS = BTNMISS;                                              //Variable used for determining current BTN being pressed

 void main(void){
    WDT_A->CTL = WDT_A_CTL_PW | WDT_A_CTL_HOLD;                         //Stop watchdog timer
    typedef enum {                                                      //New variable type for STATES
        INITIALIZE,
        WAIT_BTN,
        WAVEFORM,
        FREQUENCY,
        END_BTN,
        DUTY_CYCLE
    } STATE_TYPE;

    STATE_TYPE NS = INITIALIZE;                                         //Start at the INITIALIZE state
    while(1){                                                           //always RUN
        switch(NS){                                                     //Changes states based on variable NS (Next State)
            case INITIALIZE: {
                uint16_t i = 0, j = 0;                                  //Counting variables for wave array generation
                uint32_t voltage = 0;                                   //final voltage applied to DAC_conv (multiplied by OFFSET)
                float rad = 0;                                          //float used for sin calculations (will make startup slower)
                for(i=0;i<MAXCOUNT;i++){                                //Sine wave continuously generated to MAX
                    rad = 2*PI*j/(MAXCOUNT);                            //radians calculated by standard period div by MAX
                    voltage = DCOFFSET+DCOFFSET*sin(rad);               //final wave is 1.5+1.5*sin(x) to create 1.5 DC and 1.5 peak
                    sin_table[i] += DAC_volt_conv(voltage/OFFSET);      //set array value equal to DAC converted input value
                    j++;                                                //increment j to get next point
                }
                for(i=0;i<MAXCOUNT;i++){                                //Ramp wave must be made from two lines
                    if (i<HALFMAX){                                     //If current count is less than HALF the MAX continue
                        voltage = DCOFFSET+(MAXVOLT/MAXCOUNT)*i;        //set voltage equal to first line with y-int
                        ramp_table[i] += DAC_volt_conv(voltage/OFFSET);
                    }
                    else {                                              //run if current count is more than HALF the MAX
                        voltage = (MAXVOLT/MAXCOUNT)*(i-HALFMAX);       //set voltage equal to second line
                        ramp_table[i] += DAC_volt_conv(voltage/OFFSET);
                    }
                }
                for(i=0;i<MAXCOUNT;i++){                                //Triangle wave must be made from 3 different lines
                    if (i<QUARTERMAX){                                  //If current count is less than QUARTER the MAX continue
                        voltage = DCOFFSET+(MAXVOLT/(HALFMAX))*i;
                        tri_table[i] += DAC_volt_conv(voltage/OFFSET);
                    }
                    else if ((i>=QUARTERMAX) && (i<THREEQUARTERMAX)){   //Current count inbetween QTR and 3QTR continue
                        voltage = DC_MAXOFFSET-(MAXVOLT/(HALFMAX))*i;
                        tri_table[i] += DAC_volt_conv(voltage/OFFSET);
                    }
                    else {                                              //If current count is greater than 3QTR the MAX continue
                        voltage = (MAXVOLT/(HALFMAX))*i-DC_MAXOFFSET;
                        tri_table[i] += DAC_volt_conv(voltage/OFFSET);
                    }
                }
                keypad_init();                                          //Initialize keypad
                DAC_init();                                             //Initialize DAC
                set_DCO(FREQ_24_MHZ);                                   //SMCLK is DCOCLK divided by 1 (24 MHz)
                TIMER_A0->CCR[0] = 200;                                 //Count amount is 200 24MHz Clock Cycles
                TIMER_A0->CTL = (TIMER_A_CTL_TASSEL_2                   //Run TimerA with SMCLK
                               | TIMER_A_CTL_MC_1);                     //Count with TimerA in UP Mode
                waveform = SQUARE;                                      //Initialize the waveform as SQUARE
                frequency = F100HZ;                                     //Initialize the frequency as 100Hz
                duty_cycle = FIVE;                                      //Initialize the duty cycle as 50%
                __enable_irq();                                         //Global enable for interrupts
                NVIC->ISER[0] = (1 << TA0_0_IRQn);                      //NVIC enable for TIMER_A0
                TIMER_A0->CCTL[0] = TIMER_A_CCTLN_CCIE;                 //Enable TIMER_A0
                NS = WAIT_BTN;                                          //Switch to WAIT_BTN state
                break;
            }

            case WAIT_BTN: {
                while(BTNPRESS != BTNMISS) {                            //Wait until a button is not being pressed
                    BTNPRESS = get_key();                               //Set BTNPRESS to the current button being pressed
                }
                while(BTNPRESS == BTNMISS) {                            //Wait until a button is being pressed
                    BTNPRESS = get_key();                               //Set BTNPRESS to the current button being pressed
                }
                if ((BTNPRESS == BTN6) ||                               //Check for any buttons pressed relating to waveform
                    (BTNPRESS == BTN7) ||
                    (BTNPRESS == BTN8) ||
                    (BTNPRESS == BTN9)) {
                    NS = WAVEFORM;
                }
                else if ((BTNPRESS == BTN1) ||                          //Check for any buttons pressed relating to frequency
                         (BTNPRESS == BTN2) ||
                         (BTNPRESS == BTN3) ||
                         (BTNPRESS == BTN4) ||
                         (BTNPRESS == BTN5)) {
                    NS = FREQUENCY;
                }
                else {                                                  //Any other button continue
                    NS = DUTY_CYCLE;
                }
                break;
            }

            case WAVEFORM: {
                uint16_t i = 0;                                         //Initialize counting variable
                TIMER_A0->CCTL[0] = 0;                                  //Diable TimerA (thus the interrupts)
                if (BTNPRESS == BTN6){
                    for (i=0;i<MAXCOUNT;i++){                           //Run for the entire array
                        wave_table[i] = sin_table[i]+10;                //Set sin array to wave array for interrupt
                    }
                    waveform = SINE;                                    //Set waveform to SINE
                }
                else if (BTNPRESS == BTN7){
                    for (i=0;i<MAXCOUNT;i++){
                        wave_table[i] = tri_table[i];
                    }
                    waveform = TRIANGLE;                                //Set waveform to TRIANGLE
                }
                else if (BTNPRESS == BTN8){
                    for (i=0;i<MAXCOUNT;i++){
                        wave_table[i] = ramp_table[i];
                    }
                    waveform = RAMP;                                    //Set waveform to RAMP
                }
                else {                                                  //If nothing but square is pressed continue
                    waveform = SQUARE;                                  //Set waveform to SQUARE
                }
                TIMER_A0->CCTL[0] = TIMER_A_CCTLN_CCIE;                 //Renable TimerA
                NS = WAIT_BTN;
                break;
            }

            case FREQUENCY: {
                if (BTNPRESS == BTN1){                                  //Check for 1 being pressed
                    frequency = F100HZ;                                 //Change frequency to 100Hz
                }
                else if (BTNPRESS == BTN2){
                    frequency = F200HZ;                                 //Change frequency to 200Hz
                }
                else if (BTNPRESS == BTN3){
                    frequency = F300HZ;                                 //Change frequency to 300Hz
                }
                else if (BTNPRESS == BTN4){
                    frequency = F400HZ;                                 //Change frequency to 400Hz
                }
                else {
                    frequency = F500HZ;                                 //Change frequency to 500Hz
                }
                NS = WAIT_BTN;
                break;
            }

            case DUTY_CYCLE: {
                if (BTNPRESS == BTN0){                                  //Check for 0 being pressed
                    duty_cycle = FIVE;                                  //Set duty cycle to 50%
                }
                else if ((BTNPRESS == BTNPND) && (duty_cycle < NINE)){  //Check if PND is pressed and duty cycle less than 90%
                    duty_cycle += ONE;                                  //Increment duty cycle by 10%
                }
                else if ((BTNPRESS == BTNSTR) && (duty_cycle > ONE)){   //Check if PND is pressed and duty cycle greater than 10%
                    duty_cycle -= ONE;                                  //decrement duty cycle by 10%
                }
                NS = WAIT_BTN;
                break;
            }

            default: {                                                  //If any errors occur return to INITIALIZE state
                NS = INITIALIZE;
                break;
            }
        }
    }
}

void TA0_0_IRQHandler (void){                                           //Interrupt Handler that will occur every interrupt
    TIMER_A0->CCTL[0] &= ~TIMER_A_CCTLN_CCIFG;                          //Reset Interrupt flag for more interrupts to occur
    if (count >= MAXCOUNT-1) {                                          //If the count exceeds max continue
        count = 0;                                                      //Reset count
    }
    if (waveform != SQUARE) {                                           //If waveform is anything but a SQUARE continue
        DAC_write(wave_table[count]);                                   //Write to the DAC the current spot in the wave array
    }
    else {
        if (count*DUTY_OFFSET < (MAXCOUNT)*duty_cycle){                 //Check if count is less than duty cycle
            DAC_write(MAXDACVOLT);                                      //Write to the DAC 3V
        }
        else {
            DAC_write(MINDACVOLT);                                      //Write to the DAC 0V
        }
    }
    count += frequency;                                                 //increment the count by the frequency (1 for 100Hz)
}
