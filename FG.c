#include "msp.h"
#include "FG.h"
#include "math.h"

uint16_t DAC_volt_conv(uint16_t keypad_volts);

uint16_t sinewave_Gen(void){
    uint16_t i = 0, j = 0, sin_table[MAXCOUNT];
    uint32_t voltage = 0;
    float rad = 0;
    for(i=0;i<MAXCOUNT;i++){
        rad = 2*PI*j/(MAXCOUNT);
        voltage = 150000+150000*sin(rad);
        sin_table[i] += DAC_volt_conv(voltage/1000);
        j++;
    }
    return sin_table;
}

uint16_t trianglewave_Gen(void){
    uint16_t i = 0, tri_table[MAXCOUNT];
    uint32_t voltage = 0;
    for(i=0;i<MAXCOUNT;i++){ //Triangle wave
        if (i<QUARTERMAX){
            voltage = DCOFFSET+(MAXVOLT/HALFMAX)*i;
            tri_table[i] += DAC_volt_conv(voltage/OFFSET);
        }
        else if ((i>=QUARTERMAX) && (i<THREEQUARTERMAX)){
            voltage = DC_MAXOFFSET-(MAXVOLT/HALFMAX)*i;
            tri_table[i] += DAC_volt_conv(voltage/OFFSET);
        }
        else {
            voltage = (MAXVOLT/HALFMAX)*i-DC_MAXOFFSET;
            tri_table[i] += DAC_volt_conv(voltage/OFFSET);
        }
    }
    return tri_table;
}

uint16_t rampwave_Gen(void){
    uint16_t i = 0, ramp_table[MAXCOUNT];
    uint32_t voltage = 0;
    for(i=0;i<MAXCOUNT;i++){ //Ramp wave
        if (i<HALFMAX){
            voltage = DCOFFSET+(MAXVOLT/MAXCOUNT)*i;
            ramp_table[i] += DAC_volt_conv(voltage/OFFSET);
        }
        else {
            voltage = (MAXVOLT/MAXCOUNT)*(i-HALFMAX);
            ramp_table[i] += DAC_volt_conv(voltage/OFFSET);
        }
    }
    return ramp_table;
}


