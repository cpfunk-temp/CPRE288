#include <stdlib.h>
#include <math.h>
#include <stdio.h>
#include <timer.h>
#include <string.h>
#include "open_interface.h"
#include "lcd.h"
#include "move.h"
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
    ping_init();

    while (1)
    {
        lcd_printf("%.2f", get_ping_dist());
    }
    return 0;
}
