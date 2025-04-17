/*#include <stdlib.h>
#include <math.h>
#include <stdio.h>
#include <timer.h>
#include <string.h>
#include "open_interface.h"
#include "lcd.h"
#include "move.h"
#include "cyBot_Scan.h"
#include "uart-interrupt.h"
#include "adc.h"
#include "ping.h"

int main(void)
{
    //initializations
    timer_init();
    lcd_init();
    //adc_init(6);
    //uart_interrupt_init();
    //ping_init();
    cyBOT_init_Scan(7);
    cyBOT_Scan_t data;

    while (1)
    {
        cyBOT_Scan(180, &data);
        lcd_printf("%.2f", data.sound_dist);
    }
    return 0;
}*/
