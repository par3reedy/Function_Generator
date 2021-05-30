//Patrick Reedy; May 12, 2021

#ifndef FG_H_
#define FG_H_

#define DCOFFSET        150000                  //Value relating to 1.5V but offset for nonfloat operation
#define MAXVOLT         300000                  //Value relating to 3.0V but offset for nonfloat operation
#define DC_MAXOFFSET    450000                  //Value relating to 4.5V but offset for nonfloat operation
#define OFFSET          1000                    //Offset required to eliminate nonfloat operation offset

#define MAXCOUNT        1200                    //Amount of points in your waveform arrays
#define PI              3.14159265358979323846  //Value of PI
#define THREEQUARTERMAX (3*MAXCOUNT)/4          //3/4 of MAXCOUNT
#define HALFMAX         MAXCOUNT/2              //1/2 of MAXCOUNT
#define QUARTERMAX      MAXCOUNT/4              //1/4 of MAXCOUNT

#define MAXDACVOLT      3763                    //DAC value for 3V after error correction
#define MINDACVOLT      0                       //DAC value closest to 0V

#define SQUARE          1                       //Value associated with SQUARE
#define SINE            2
#define TRIANGLE        3
#define RAMP            4

#define DUTY_OFFSET     10                      //Since we want no float operations in ISR need offset
#define FIVE            5                       // so duty cycles are actual percents or portions of count
#define ONE             1                       //Value for 10% duty cycle
#define NINE            9

#define F100HZ          1                       //Value for 100Hz
#define F200HZ          2
#define F300HZ          3
#define F400HZ          4
#define F500HZ          5

#endif /* FG_H_ */
