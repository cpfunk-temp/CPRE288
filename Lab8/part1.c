/*#include <stdlib.h>
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

int main(void)
{
    //initializations
    timer_init();
    lcd_init();
    uart_interrupt_init();
    adc_init();
    oi_t *sensor_data = oi_alloc();
    oi_init(sensor_data);
    char txt_buffer[200];
    uint16_t data;

    while (1)
    {
        data = adc_read();
        lcd_printf("%d", data);
        snprintf(txt_buffer, 200, "%d\r\n", data, );
        uart_sendStr(txt_buffer);
        timer_waitMillis(100);
    }

    return 0;
}*/
