#include <inc/tm4c123gh6pm.h>
#include <stdint.h>
#include "button.h"
#include "timer.h"
#include "lcd.h"

#include "servo.h"

//calibration values
volatile uint32_t right_calibration_value = 16000;
volatile uint32_t left_calibration_value = 32000;

void set_servo_calibration(uint32_t left_val, uint32_t right_val)
{
    right_calibration_value = right_val;
    left_calibration_value = left_val;
}

void servo_init()
{
    //enable clock to GPIO Port B
    SYSCTL_RCGCGPIO_R |= 0x2;

    //enable clock to timer 1
    SYSCTL_RCGCTIMER_R |= 0x2;

    //wait for Port B and timer 1 to be ready
    while ((SYSCTL_PRGPIO_R & 0x2) == 0);
    while ((SYSCTL_PRTIMER_R & 0x2) == 0);

    //set GPIO PB5 to output
    GPIO_PORTB_DIR_R |= 0x20;
    //enable digital functions for GPIO Port B
    GPIO_PORTB_DEN_R |= 0x20;
    //enable alternative function for PB5
    GPIO_PORTB_AFSEL_R |= 0x20;
    //set timer as alternative function for PB5
    GPIO_PORTB_PCTL_R = (GPIO_PORTB_PCTL_R & ~0xF00000) | 0x700000;

    //disable GPTM timer before configuring
    TIMER1_CTL_R &= ~0x100;
    //GPTM configuration: 16-bit mode
    TIMER1_CFG_R = 0x4;
    //configure GPTM Timer Mode
    //enable PWM mode
    TIMER1_TBMR_R |= 0x8;
    //disable edge time mode
    TIMER1_TBMR_R &= ~0x4;
    //enable periodic time mode
    TIMER1_TBMR_R = (TIMER1_TBMR_R & ~0x3) | 0x2;
    //PWM signal is not inverted
    TIMER1_CTL_R &= ~0x4000;
    //load PWM period into Timer B Interval Load register
    //20e-3s * 16MHz = 320,000 (0x4E200) cycles per 20ms period
    TIMER1_TBPR_R = 0x0004;
    TIMER1_TBILR_R = 0x0000E200;
    //load PWM with a duty cycle of 0%
    TIMER1_TBPMR_R = 0x0;
    TIMER1_TBMATCHR_R = 0x0;
    //enable GPTM timer
    TIMER1_CTL_R |= 0x100;
}

uint32_t angle_to_cycles(int32_t degrees)
{
    uint32_t m = (left_calibration_value - right_calibration_value) / 180;
    return 320000 - (degrees * m + right_calibration_value);
}

void turn_servo_to(int16_t degrees)
{
    uint32_t cycles = angle_to_cycles(degrees);
    TIMER1_TBPMR_R = (cycles >> 16) & 0xFF;
    TIMER1_TBMATCHR_R = cycles & 0xFFFF;
}

void cyBOT_SERVO_cal()
{
    //initializations
    timer_init();
    lcd_init();
    button_init();
    servo_init();
    uint32_t left_value;
    uint32_t right_value;

    int16_t angle = 0;
    uint8_t btn = 0;
    uint8_t next = 0;

    while (!next)
    {
        btn = button_getButton();
        switch (btn)
        {
        case 1:
            angle--;
            break;
        case 2:
            angle++;
            break;
        case 4:
            next = 1;
            right_value = 320000 - angle_to_cycles(angle);
            break;
        }
        lcd_printf("right value: %d", 320000 - angle_to_cycles(angle));
        turn_servo_to(angle);
        timer_waitMillis(5);
    }
    while (next != 0)
    {
        next = button_getButton();
    }
    next = 0;
    angle = 180;
    while (!next)
    {
        btn = button_getButton();
        switch (btn)
        {
        case 1:
            angle--;
            break;
        case 2:
            angle++;
            break;
        case 4:
            next = 1;
            left_value = 320000 - angle_to_cycles(angle);
            break;
        }
        lcd_printf("left value: %d", 320000 - angle_to_cycles(angle));
        turn_servo_to(angle);
        timer_waitMillis(5);
    }

    while (1)
    {
        lcd_printf("left value: %d\nright value: %d\n", left_value, right_value);
    }
}

