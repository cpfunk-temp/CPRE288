/**
 * lab6-interrupt_template.c
 *
 * Template file for CprE 288 Lab 6
 *
 * @author Diane Rover, 2/15/2020
 *
 */

#include "Timer.h"
#include "lcd.h"
#include "cyBot_Scan.h"  // For scan sensors
#include "uart-interrupt.h"

// Uncomment or add any include directives that you want to use
// #include "open_interface.h"
// #include "movement.h"
// #include "button.h"

// Your code can use the global variables defined in uart-interrupt.c
// They are declared with the extern qualifier in uart-interrupt.h, which makes the variables visible to this file.

int main(void) {
    timer_init(); // Must be called before lcd_init(), which uses timer functions
    lcd_init();
    uart_interrupt_init();
    cyBOT_init_Scan(7);
    //cyBOT_SERVO_cal();
    right_calibration_value = 290500;
    left_calibration_value = 1261750;

    cyBOT_Scan_t scanData;

    //wait for command flag
    set_cmnd('g');
    while(!command_flag);

    set_cmnd('s');
    uint8_t i;
    for (i = 0; i <= 180; i += 2)
    {

        if (command_flag)
        {
            set_cmnd('g');
            while(!command_flag);
            set_cmnd('s');
        }

        cyBOT_Scan(i, &scanData);
    }
}
