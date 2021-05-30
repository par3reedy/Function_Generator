//Patrick Reedy and Jake Myers; April 26, 2021
/*This header file describes values pertaining to the operation of TIMERA within the
MSP432P401R Microprocessor from Texas Instruments. */

#ifndef TIMERA_H_
#define TIMERA_H_

#define NEWCLKOUTPORT   P5          //Port used to output new CLK signal derived from MCLK
#define NEWCLKOUTPIN    BIT0        //Pin used in correlation with PORT for new CLK
#define MCLKPORT        P4          //Port used to describe MCLK
#define MCLKPIN         BIT3        //Pin used in correlation with PORT for MCLK

#endif /* TIMERA_H_ */
