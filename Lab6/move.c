#include <stdlib.h>
#include <stdint.h>
#include "open_interface.h"

#include "move.h"

uint16_t spd = 150;

void set_speed(uint16_t speed)
{
    spd = speed;
}

float turn_left(oi_t* sensor_data, float degrees)
{
    float start_angle = 0;

    oi_setWheels(spd, -spd);

    while (start_angle < degrees)
    {
        oi_update(sensor_data);
        start_angle += sensor_data->angle;

    }

    oi_setWheels(0, 0);

    return start_angle;
}

float turn_right(oi_t* sensor_data, float degrees)
{
    float start_angle = 0;

    oi_setWheels(-spd, spd);

    while (start_angle < degrees)
    {
        oi_update(sensor_data);
        start_angle -= sensor_data->angle;
    }

    oi_setWheels(0, 0);

    return start_angle;
}

float move_forward(oi_t *sensor_data, float distance_mm, uint8_t (*stop_event)(oi_t*))
{
    float sum = 0;
    oi_setWheels(spd,spd);
	
    while (sum < distance_mm && !(*stop_event)(sensor_data))
	{
        oi_update(sensor_data);
        sum += sensor_data->distance;
    }
    //stop robot
    oi_setWheels(0,0);

    return sum;
}

float move_backward(oi_t *sensor_data, float distance_mm)
{
    float sum = 0;
    oi_setWheels(-spd,-spd);
    while (sum < distance_mm)
	{
        oi_update(sensor_data);
        sum -= sensor_data->distance;
    }
    //stop robot
    oi_setWheels(0,0);

    return sum;
}
