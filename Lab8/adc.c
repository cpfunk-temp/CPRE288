#include <math.h>
#include <stdio.h>
#include <timer.h>
#include <string.h>
#include "open_interface.h"
#include "lcd.h"
#include "move.h"
#include "cyBot_uart.h"
#include "uart-interrupt.h"
#include "adc.h"

void adc_init(uint8_t oversampling)
{

    //initialize analog functionality for PB4
    SYSCTL_RCGCADC_R |= 0x1; //activate clock 0 for ADC
    SYSCTL_RCGCGPIO_R |= 0x2; //activate Port B clock
    GPIO_PORTB_DEN_R &= ~0x10; //disable digital functionality
    GPIO_PORTB_AMSEL_R |= 0x10; //enable analog functionality
    while ((SYSCTL_PRGPIO_R & 0x2) == 0); //wait to proceed until GPIO timer is ready
    GPIO_PORTB_DIR_R &= ~0x10; //set PB4 as input
    GPIO_PORTB_AFSEL_R |= 0x10; //set alternative function for Port B
    while ((SYSCTL_PRADC_R & 0x1) == 0); //wait to proceed until ADC module is ready
    ADC0_PC_R = (ADC0_PC_R & ~0xF) | 0x1; //set number of samples per second to lowest value 125K samples per sec
    if (oversampling > 0 && oversampling < 7) //enable oversampling
    {
        //the hardware takes 2^oversampling samples and averages them
        ADC0_SAC_R = oversampling;
    }
    else
    {
        ADC0_SAC_R = 0x0;
    }
    ADC0_SSPRI_R = 0x3210; //make sure sample sequencer 0 is highest priority
    ADC0_ACTSS_R &= ~0x1; //disable sample sequencer 0
    ADC0_EMUX_R &= ~0xF; //configure processor trigger for sample sequencer 0
    ADC0_SSMUX0_R = (ADC0_SSMUX0_R & ~0xF) | 0xA; //set ADC channel to channel 10 which is PB4
    ADC0_SSCTL0_R = 0x00000006;
    ADC0_IM_R &= ~0x1; //disable sample sequencer 0 interrupts
    ADC0_ACTSS_R |= 0x1; //enable sample sequencer 0
}

uint16_t adc_read()
{
    int16_t data;
    ADC0_PSSI_R = 0x1; //028

    while((ADC0_RIS_R & 0x1) == 0); //wait for raw interrupt status flag

    data = ADC0_SSFIFO0_R; //get data

    ADC0_ISC_R = 0x1; //clear interrupt status

    return data;
}
