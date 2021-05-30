//Patrick Reedy; May 12, 2021
//DAC Data Sheet: https://ww1.microchip.com/downloads/en/DeviceDoc/22248a.pdf

/*--------------------------------------------------------------------------------------------------------------------------------------
*  Library Definitions
--------------------------------------------------------------------------------------------------------------------------------------*/
#include "msp.h"
#include "dac.h"
#include "DCO.h"

/*--------------------------------------------------------------------------------------------------------------------------------------
*  Function Definitions
--------------------------------------------------------------------------------------------------------------------------------------*/

//Function to Initialize the DAC with SPI
void DAC_init(void){
    EUSCI_B0->CTLW0 |= EUSCI_B_CTLW0_SWRST;         //Set EUSCIB to software reset

    EUSCI_B0->CTLW0 = (EUSCI_B_CTLW0_MSB            //Send MSB first
                     | EUSCI_B_CTLW0_MST            //Controller Mode Enabled (DAC peripheral)
                     | EUSCI_B_CTLW0_MODE_0         //3-Pin SPI Mode
                     | EUSCI_B_CTLW0_SYNC           //Synchronous Mode
                     | EUSCI_B_CTLW0_SSEL__SMCLK    //Clock out is SMCLK
                     | EUSCI_B_CTLW0_SWRST          //Renable Software Reset
                     | EUSCI_B_CTLW0_CKPH);         //Clock Phase On (changes data on falling edge)

    EUSCI_B0->BRW = 0x01;                           //Divide Clock by 1

    SPI_PORT->SEL0 |= (SPI_SCK | SPI_COPI);         //SPI Clock and COPI set up for Primary Module Function
    SPI_PORT->SEL1 &= ~(SPI_SCK | SPI_COPI);

    SPI_PORT->SEL0 &= ~SPI_CS;                      //Chip Select set as GPIO
    SPI_PORT->SEL1 &= ~SPI_CS;
    SPI_PORT->DIR |= SPI_CS;                        //Chip Select as Output
    SPI_PORT->OUT |= SPI_CS;                        //Chip Select works on active Low, so initialize high

    EUSCI_B0->CTLW0 &= ~(EUSCI_B_CTLW0_SWRST);      //Clear Software Reset
}

//Function to write to the DAC
void DAC_write(uint16_t data){
    uint8_t loByte, hiByte;                         //Initialize the lower and upper bytes
    data |= (GA | SHDN);                            //Append the DATA with upper config bits
                                                    // where Gain is 1 and SHDN is 1.
    loByte = data & 0xFF;                           //Mask Lower Byte
    hiByte = (data >> 8) & 0xFF;                    //Mask Upper Byte

    SPI_PORT->OUT &= ~(SPI_CS);                     //Set Chip Select LOW

    while(!(EUSCI_B0->IFG & EUSCI_B_IFG_TXIFG));    //Wait until the sending buffer is empty
    EUSCI_B0->TXBUF = hiByte;                       //Hibyte is sent to the buffer

    while(!(EUSCI_B0->IFG & EUSCI_B_IFG_TXIFG));    //Wait until the sending buffer is empty
    EUSCI_B0->TXBUF = loByte;                       //Lobyte is sent to the buffer

    while(!(EUSCI_B0->IFG & EUSCI_B_IFG_RXIFG));    //Wait until the reading buffer is empty
    SPI_PORT->OUT |= SPI_CS;                        //Set chip select high after data transmission
}

//Function for converting user input to DAC input data
uint16_t DAC_volt_conv(uint16_t keypad_volts){
    uint32_t optimization;                          //Variable used for Optimizing
    uint16_t volts;                                 //Variable used to truncate optimized value for sending
    if (keypad_volts >= 330){                       //If user input is greater than 330 set output to max voltage
        return 4095;                                //Max voltage of 3.3 V is 4095 DAC input data
    }
    else if (keypad_volts == 0){                    //If user input is 0 set output to lowest voltage
        return 0;                                   //Lowest voltage of 0 is 0 DAC input data
    }
    else {                                          //If user input is within bounds
        optimization = (12467*keypad_volts-659);    //Optimization Function derived from linear optimization
        volts = optimization/1000;                  //Convert back to units for DAC
        return volts;
    }
}
