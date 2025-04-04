#include <stdlib.h>
#include "open_interface.h"
#include "lcd.h"
#include "move.h"

int main(void)
{
    lcd_init();
    oi_t *sensor_data = oi_alloc();
    oi_init(sensor_data);

    set_speed(200);
    float dist = move_forward(sensor_data, 2000);
    lcd_printf("%f", dist);

    oi_free(sensor_data);
	return 0;
}
