//Patrick Reedy; May 12, 2021

#ifndef DAC_H_
#define DAC_H_

#define SPI_CS      BIT7                    //Pin 7 used for Chip Select
#define SPI_COPI    BIT6                    //Pin 6 used for Controller Out Peripheral In
#define SPI_SCK     BIT5                    //Pin 6 used for DAC Clock
#define SPI_PORT    P1                      //Port used for SPI

#define BUF         BITE                    //Buffer Setting
#define GA          BITD                    //Gain Setting
#define SHDN        BITC                    //Shut Down Setting

#define LED         (BIT0 | BIT1 | BIT2)    //LED on Pins 0-2
#define LEDPORT     P2                      //LED Port
#define RED         0x01                    //Value associated with LED color RED
#define GREEN       0x02                    //Green
#define BLUE        0x04                    //Blue
#define WHITE       0x07                    //White

#endif /* DAC_H_ */
