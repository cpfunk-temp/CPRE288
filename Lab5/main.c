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
 */

#include "button.h"
#include "timer.h"
#include "lcd.h"
#include "uart.h"

#include "cyBot_uart.h"  // Functions for communicating between CyBot and Putty (via UART1)
                         // PuTTy: Baud=115200, 8 data bits, No Flow Control, No Parity, COM1
#include "cyBot_Scan.h"  // Scan using CyBot servo and sensors

void clear_dataBuffer(char* data_buffer,uint8_t size)
{
    uint8_t i;
    for (i = 0; i < size; i++)
    {
        data_buffer[i] = 0;
    }
}

void insert_data(char* data_buffer, uint8_t* data_cnt, uint8_t size, char data)
{
    if (*data_cnt < size)
    {
        (*data_cnt)++;
        data_buffer[(*data_cnt) - 1] = data;
    }
    else
    {
        uint8_t i;
        for (i = 0; i < size - 1; i++)
        {
            data_buffer[i] = data_buffer[i + 1];
        }
        data_buffer[size - 1] = data;
    }
}

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

    //initialize values in data buffer to 0
    clear_dataBuffer(data_buffer, 21);

    while(1)
    {
        data = uart_receive();

        if (data == '\r')
        {
            data_cnt = 0;
            clear_dataBuffer(data_buffer, 21);
            lcd_printf("%s\n%d", data_buffer, data_cnt);
            snprintf(txt_buffer, 50, "%s\n\r%d\n\r", data_buffer, data_cnt);
            uart_sendStr(txt_buffer);
        }
        else
        {
            insert_data(data_buffer, &data_cnt, 20, data);
            lcd_printf("%s\n%d", data_buffer, data_cnt);
            snprintf(txt_buffer, 50, "%s\n\r%d\n\r", data_buffer, data_cnt);
            uart_sendStr(txt_buffer);
        }
    }

}
