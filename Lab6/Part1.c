/**
 * lab5_template.c
 *
 * Template file for CprE 288 Lab 5
 *
 * @author Zhao Zhang, Chad Nelson, Zachary Glanz
 * @date 08/14/2016
 *
 * @author Phillip Jones, updated 6/4/2019
 * @author Diane Rover, updated 2/25/2021, 2/17/2022
 *

#include "button.h"
#include "timer.h"
#include "lcd.h"
#include "uart.h"

#include "cyBot_uart.h"  // Functions for communicating between CyBot and Putty (via UART1)
                         // PuTTy: Baud=115200, 8 data bits, No Flow Control, No Parity, COM1
#include "cyBot_Scan.h"  // Scan using CyBot servo and sensors

int main(void) {
    button_init();
    timer_init(); // Must be called before lcd_init(), which uses timer functions
    lcd_init();

    // initialize the cyBot UART1 before trying to use it
    uart_init();

    cyBOT_init_Scan(7);
    //cyBOT_SERVO_cal();
    right_calibration_value = 290500;
    left_calibration_value = 1261750;
    // Initialize the scan

    cyBOT_Scan_t scanData;

    char data;
    char txt_buffer[200];

    uint8_t data_cnt = 0;
    char data_buffer[21];

    while(data != 'g')
    {
        data = uart_receive();
    }

    uint8_t i;
    for (i = 0; i <= 180; i += 2)
    {
        data = uart_receive_noBlock();
        if (data == 's')
        {
            while(data != 'g')
            {
                data = uart_receive();
            }
        }
        cyBOT_Scan(i, &scanData);
    }
}
*/
