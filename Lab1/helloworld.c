#include "Timer.h"
#include "lcd.h"
#include "stdlib.h"
#include "string.h"

void moveBanner(int startPos, char* message, int messageLen)
{
	int i;
	int j = 0;

	for (i = 0; i < 20; i++)
	{
		if (i < startPos)
		{
			lcd_putc(message[messageLen - startPos + i]);
		}
		else
		{
			lcd_putc(message[j]);
			j++;
		}
	}
}


int main (void) {

	timer_init(); // Initialize Timer, needed before any LCD screen functions can be called
	              // and enables time functions (e.g. timer_waitMillis)
	lcd_init();   // Initialize the LCD screen.  This also clears the screen. 
	char message[] = "                    Microcontrollers are lots of fun!";
	int i;
	while (1)
	{
		for (i = strlen(message); i >= 0 ; i--)
		{
			moveBanner(i, message, strlen(message));
			timer_waitMillis(300);
			lcd_init();
		}
	}

    
	// NOTE: It is recommended that you use only lcd_init(), lcd_printf(), lcd_putc, and lcd_puts from lcd.h.
	// NOTE: For time functions, see Timer.h

	return 0;
}
