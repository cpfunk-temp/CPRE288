/*#include <stdlib.h>
#include "open_interface.h"
#include "lcd.h"
#include "move.h"
#include "putty.h"
#include "cyBot_uart.h"
#include "cyBot_Scan.h"
#include <math.h>
#include <stdio.h>
#include <timer.h>
#include <string.h>

void putty_puts(const char* str)
{
    while (*str != '\0')
    {
        cyBot_sendByte(*str);
        str++;
    }
}

//this function is calibrated using a proportionality constant
float prop_const = 0.886094;
float obj_width_PingCalibrated(short delta_angle, float ping_dist)
{
    return 2*ping_dist*tan(abs(delta_angle)*M_PI/360) - prop_const*ping_dist;
}


// find the angles where the robot should start scanning
void calibrate_width(short cm_ping_cutoff, float actual_width)
{
    cyBOT_init_Scan(7);
    //cyBOT_SERVO_cal();

    right_calibration_value = 311500;
    left_calibration_value = 1251250;
    //putty connection
    cyBot_uart_init();

    //variables
    char buffer[200];
    cyBOT_Scan_t scan;

    // Initialization steps
    cyBOT_Scan(90, &scan);
    //wait for the servo to sweep to zero
    timer_waitMillis(500);
    //if you remove this part, the field will be "scanned" twice

    // End of initialization steps

    //clear lcd
    lcd_printf("");

    //scan 180 degrees
    uint8_t angle;
    cyBOT_Scan(90, &scan);
    lcd_printf("%d", scan.IR_raw_val);
    snprintf(buffer, 200, "%d\n\r", scan.IR_raw_val);
    putty_puts(buffer);
}

int main(void)
{
    //initializations
    timer_init();
    lcd_init();
    oi_t *sensor_data = oi_alloc();
    oi_init(sensor_data);
    cyBot_uart_init();
    char buffer[200];
    snprintf(buffer, 200, "data:\n\r");
    putty_puts(buffer);
    short i;
    for (i = 0; i < 15; i++)
    {
        if (i != 0) move_backward(sensor_data, 80);
        calibrate_width(100, 5);
    }

    oi_free(sensor_data);
	return 0;
}*/
