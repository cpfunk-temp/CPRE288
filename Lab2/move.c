#include <stdlib.h>
#include <stdint.h>
#include "open_interface.h"

#include "move.h"

enum bump_directions
{
    LEFT = 1,
    RIGHT = 2
};

uint16_t spd = 150;

void set_speed(uint16_t speed)
{
    spd = speed;
}

uint8_t getBumpDirection(oi_t* sensor_data)
{
    if (sensor_data->bumpLeft)
    {
        return LEFT;
    }
    else if (sensor_data->bumpRight)
    {
        return RIGHT;
    }

    return 0;
}

void on_collision(oi_t* sensor_data, uint8_t bump_direction)
{
    move_backward(sensor_data, 150);

    switch (bump_direction)
    {
    case LEFT:
        turn_right(sensor_data, 90);
        break;
    case RIGHT:
        turn_left(sensor_data, 90);
        break;
    }

    move_forward(sensor_data, 250);

    switch (bump_direction)
    {
    case LEFT:
        turn_left(sensor_data, 90);
        break;
    case RIGHT:
        turn_right(sensor_data, 90);
        break;
    }

    move_forward(sensor_data, 150);
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

float move_forward(oi_t *sensor_data, float distance_mm)
{
    float sum = 0;
    uint8_t bump_direction = 0;
    oi_setWheels(spd,spd);
    while (sum < distance_mm)
	{
        oi_update(sensor_data);
        sum += sensor_data->distance;
        bump_direction = getBumpDirection(sensor_data);
        if (bump_direction)
        {
            on_collision(sensor_data, bump_direction);
            oi_setWheels(spd,spd);
        }
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

