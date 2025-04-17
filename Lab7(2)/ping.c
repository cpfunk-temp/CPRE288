/**
 * Driver for ping sensor
 * @file ping.c
 * @Christopher Funk
 */

#include "ping.h"
#include "Timer.h"
#include "lcd.h"

// Global shared variables
// Use extern declarations in the header file

volatile uint32_t g_start_time = 0;
volatile uint32_t g_end_time = 0;
volatile enum{LOW, HIGH, DONE} g_state = LOW;

volatile uint8_t overflow = 0; //FIXME for debugging only

void ping_init()
{
    //enable clock to GPIO Port B
    SYSCTL_RCGCGPIO_R |= 0x2;

    //enable clock to timer 3
    SYSCTL_RCGCTIMER_R |= 0x8;

    //wait for Port B and Timer 3 to be ready
    while ((SYSCTL_PRGPIO_R & 0x2) == 0);
    while ((SYSCTL_PRTIMER_R & 0x8) == 0);

    //set GPIO PB3 to input
    GPIO_PORTB_DIR_R &= ~0x8;
    //enable digital functions for GPIO Port B
    GPIO_PORTB_DEN_R |= 0x8;
    //enable alternative function for PB3
    GPIO_PORTB_AFSEL_R |= 0x8;
    //set timer as alternative function for PB3
    GPIO_PORTB_PCTL_R = (GPIO_PORTB_PCTL_R & ~0xF000) | 0x7000;

    //disable TBEN timer before configuring
    TIMER3_CTL_R &= ~0x100;
    //GPTM configuration: 16-bit mode
    TIMER3_CFG_R = 0x4;
    //configure GPTM Timer Mode
    TIMER3_TBMR_R |= 0x4;//edge-time mode
    TIMER3_TBMR_R |= 0x3;//capture mode
    TIMER3_TBMR_R &= ~0x10;//count down
    //type of event captured
    TIMER3_CTL_R |= 0xC00;//event: rising and falling edges
    //timer B interval load
    TIMER3_TBILR_R = 0x0000FFFF;
    //load prescaler
    TIMER3_TBPR_R = 0x00FF;
    //set interrupt types
    TIMER3_IMR_R |= 0x400;

    //NVIC setup: set priority to 1
    NVIC_PRI9_R = (NVIC_PRI9_R & ~0xE0) | 0x20;//bits 15:13
    //NVIC setup: arm timer 3B interrupt
    NVIC_EN1_R |= 0x10;
    IntRegister(INT_TIMER3B, TIMER3B_Handler);

    IntMasterEnable();

    //enable GPTM timer
    TIMER3_CTL_R |= 0x100;
}

void ping_trigger()
{
    //Disable timer 3B
    TIMER3_CTL_R &= ~0x100;
    //disable CBEIM interrupt
    TIMER3_IMR_R &= ~0x400;
    //disable alternate function (disconnect timer from port pin)
    GPIO_PORTB_AFSEL_R &= ~0x8;
    //disable alternative function for PB3
    GPIO_PORTB_PCTL_R &= ~0xF000;
    //switch PB3 to output
    GPIO_PORTB_DIR_R |= 0x8;

    //send 5us pulse
    GPIO_PORTB_DATA_R |= 0x8;
    timer_waitMicros(5);
    GPIO_PORTB_DATA_R &= ~0x8;

    //switch PB3 back to input
    GPIO_PORTB_DIR_R &= ~0x8;
    //set timer as alternative function for PB3
    GPIO_PORTB_PCTL_R |= 0x7000;
    //re-enable alternate function, timer interrupt, and timer
    GPIO_PORTB_AFSEL_R |= 0x8;
    //clear an interrupt that may have been erroneously triggered
    TIMER3_ICR_R |= 0x400;
    //enable CBEIM interrupt
    TIMER3_IMR_R |= 0x400;
    //reload start values in timer
    TIMER3_TBILR_R = 0x0000FFFF;
    TIMER3_TBPR_R = 0x00FF;
    //enable timer 3B
    g_state = LOW;
    TIMER3_CTL_R |= 0x100;
}

void TIMER3B_Handler(void)
{
    //check if handler called due to edge time event
    if (TIMER3_MIS_R & 0x400)
    {
        //clear the trigger flag (clear by writing 1 to ICR)
        TIMER3_ICR_R |= 0x400;

        switch(g_state)
        {
        case LOW:
			//read from TIMER3_TBR_R rather than TIMER3_TBV_R because
			//this register contains the time since the last edge event
            g_start_time = TIMER3_TBR_R;
            g_state = HIGH;
            break;
        case HIGH:
            g_end_time = TIMER3_TBR_R;
            g_state = DONE;
            break;
        }
    }
}

float get_ping_dist()
{
    float spd_of_sound = 34300.0;
    float clock_period = 62.5e-9;//period based on a 16MHz clock
    //trigger pulse
    ping_trigger();
    //wait until pulse is done (longest pulse takes ~20ms)
    timer_waitMillis(20);

    //check for timer overflow
    if (g_end_time > g_start_time)//FIXME for debugging only
    {
        overflow++;//FIXME for debugging only
        lcd_printf("%d", overflow);//FIXME for debugging only
        timer_waitMillis(500);//FIXME for debugging only
    }

    //return distance if pulse finished and no overflow occurred  
    if (g_state != DONE || g_end_time > g_start_time)
    {
        return -1;
    }
    else
    {
        return (g_start_time - g_end_time) * clock_period * spd_of_sound * 0.5;
    }
}
