#include "button.h"
#include "Timer.h"
#include "lcd.h"
#include "cyBot_uart.h"
#include "send_str.h"
#include <inc/tm4c123gh6pm.h>


int main(void) {
    button_init();
    timer_init();
    lcd_init();
    cyBot_uart_init();

    char txt_buffer[50];
    uint8_t btn_press;
    uint8_t btn_old;

    lcd_printf("%x", UART1_CTL_R);  //FIXME for debug only
    /*
    while(1)
    {
        btn_press = button_getButton();

        if (btn_press != btn_old && btn_press != 0)
        {
            lcd_printf("Button pressed: %d", btn_press);
            snprintf(txt_buffer, 50, "Button pressed: %d\n\r", btn_press);
            putty_puts(txt_buffer);
            btn_old = btn_press;
        }
		if (btn_press == 0)
		{
		    btn_old = 0;
            lcd_printf("");
		}
    }*/
}
