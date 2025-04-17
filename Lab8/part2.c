#include <stdlib.h>
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
#include "cyBot_uart.h"
#include "cyBot_Scan.h"
#include "button.h"

/* TODO:
 * implement ir to distance conversion function
 */

float ir_to_dist(uint16_t ir_val)
{
    float m = 39248;
    float b = -4.92824;
    return m / ir_val + b;
}

float batt_percent_charge(oi_t* sensor_data)
{
    return (float)(sensor_data->batteryCharge) / (sensor_data->batteryCapacity);
}

void display_charge(oi_t *sensor_data)
{
    oi_update(sensor_data);

    lcd_printf("CHARGE:%.1f%%", batt_percent_charge(sensor_data) * 100);

    timer_waitMillis(1000);
}

/*int main(void)
{
    //initializations
    timer_init();
    lcd_init();
    button_init();
    //REMOVE \/\/\/\/\/\/\/ this and add lab 9 functions

    cyBOT_Scan_t scan;
    // Initialization steps
    cyBOT_init_Scan(7);
    //cyBOT_SERVO_cal();
    right_calibration_value = 243250;
    left_calibration_value = 1225000;
    //REMOVE^^^^^^^^^^^^^^^^^^^^^^^^^^^^
    uart_interrupt_init();
    adc_init(6);
    //oi_interface stuff
    oi_t *sensor_data = oi_alloc();
    oi_init(sensor_data);

    display_charge(sensor_data);
    //other
    char txt_buffer[200];
    uint16_t data;

    uint8_t spdy = 20;
    while (button_getButton() == 0);
    oi_setWheels(0, 0);
    uint8_t i;
    for (;;) //collect data points
    {
        while (button_getButton() == 0);
        cyBOT_Scan(90, &scan);//FIXME REMOVE and add Lab 9 functions
        data = adc_read();
        lcd_printf("%d", data);
        snprintf(txt_buffer, 200, "%d\r\n", data);
        uart_sendStr(txt_buffer);
    }
    oi_setWheels(0, 0);

    oi_free(sensor_data);
    return 0;
}*/

int main(void)
{
    //initializations
    timer_init();
    lcd_init();
    button_init();
    //REMOVE \/\/\/\/\/\/\/ this and add lab 9 functions

    //cyBOT_Scan_t scan;
    // Initialization steps
    //cyBOT_init_Scan(7);
    //cyBOT_SERVO_cal();
    right_calibration_value = 243250;
    left_calibration_value = 1225000;
    //REMOVE^^^^^^^^^^^^^^^^^^^^^^^^^^^^
    uart_interrupt_init();
    adc_init(6);
    //oi_interface stuff
    //oi_t *sensor_data = oi_alloc();
    //oi_init(sensor_data);

    //display_charge(sensor_data);
    //other
    char txt_buffer[200];
    uint16_t data;

    uint8_t spdy = 20;
    while (button_getButton() == 0);
    //oi_setWheels(0, 0);
    uint8_t i;
    for (;;) //collect data points
    {
        //cyBOT_Scan(90, &scan);//FIXME REMOVE and add Lab 9 functions
        data = adc_read();
        lcd_printf("%d %.2f", data, ir_to_dist(data));
        snprintf(txt_buffer, 200, "%d %.2f\r\n", data, ir_to_dist(data));
        uart_sendStr(txt_buffer);
        timer_waitMillis(100);

    }
    //oi_setWheels(0, 0);
    return 0;
}


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

    right_calibration_value = 222250;
    left_calibration_value = 1183000;
    //putty connection
    cyBot_uart_init();

    //variables
    //ping values
    char buffer[200];
    short r_pingEdge = 0;
    short l_pingEdge = 0;
    float ping_dist;

    float objWidth;
    uint8_t angle_delta;
    //arrays to store raw data
    //short ping_dists[91];
    //short ir_vals[91];

    uint8_t objCnt = 0;

    cyBOT_Scan_t scan;

    // Initialization steps
    cyBOT_Scan(0, &scan);
    //wait for the servo to sweep to zero
    timer_waitMillis(500);
    //if you remove this part, the field will be "scanned" twice

    // End of initialization steps

    //clear lcd
    lcd_printf("");

    //scan 180 degrees
    uint8_t angle;
    for (angle = 0; angle <= 180; angle += 2)
    {
        cyBOT_Scan(angle, &scan);

        //check for items lower than cutoff
        if (!r_pingEdge && scan.sound_dist < cm_ping_cutoff)
        {
            r_pingEdge = angle;
            //snprintf(buffer, 200, "%d\n\r", 1);                 //FIXME for debugging purposes only
            //putty_puts(buffer);
            ping_dist = scan.sound_dist;
        }
        else if (r_pingEdge && scan.sound_dist > cm_ping_cutoff)
        {
            //snprintf(buffer, 200, "%d\n\r", -1);                //FIXME for debugging purposes only
            //putty_puts(buffer);
            l_pingEdge = angle;
        }
        else                                                    //FIXME for debugging purposes only
        {
            //snprintf(buffer, 200, "%d\n\r", !!r_pingEdge);
            //putty_puts(buffer);
        }

        //populate ir and ping sensor array
        //ping_dists[angle/2] = (short)(scan.IR_raw_val);         //FIXME for debugging purposes only
        //ir_vals[angle/2] = (short)(scan.sound_dist);

        // if both left and right edges are detected, find object width
        // only allow objects to be registered if the angle in which the object is detected is at a minimum "width" of 6 degrees
        angle_delta = abs(l_pingEdge - r_pingEdge);
        if (l_pingEdge && r_pingEdge && angle_delta < 6)
        {
            //reset both angles to zero, no object detected
            r_pingEdge = 0;
            l_pingEdge = 0;
        }
        else if (l_pingEdge && r_pingEdge)
        {
            objWidth = obj_width_PingCalibrated(l_pingEdge - r_pingEdge, ping_dist);
            lcd_printf("%.2f, %.2f, %.2f", objWidth, fabs(objWidth - actual_width), ping_dist);
            snprintf(buffer, 200, "%.2f, %.2f, %.2f\n\r", objWidth, fabs(objWidth - actual_width), ping_dist);
            putty_puts(buffer);

            objCnt++;
            //reset both angles to zero
            r_pingEdge = 0;
            l_pingEdge = 0;

            timer_waitMillis(300);
            return;
        }
    }
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
    for (i = 0; i < 5; i++)
    {
        if (i != 0) move_backward(sensor_data, 50);
        calibrate_width(100, 5);
    }

    oi_free(sensor_data);
    return 0;
}*/
