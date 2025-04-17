#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#include "uart-interrupt.h"
#include "lcd.h"
#include "open_interface.h"

#include "move.h"

#define TO_RAD(x) (x*M_PI/180.0f)
#define TO_DEG(x) (x*180.0f/M_PI)

#define RIGHT 0
#define LEFT 1

typedef struct
{
    float i;
    float j;
} vector_2D;

uint16_t spd = 150;

//NOTE: distances are stored in cm and angles are stored in degrees
volatile coord target_coord = {0, 0};
volatile coord current_coord = {0, 0};
volatile float current_angle = 90;

//returns distance to target in cm
float dist_to_target()
{
    vector_2D dist_vect;
    dist_vect.i = target_coord.x - current_coord.x;//cm
    dist_vect.j = target_coord.y - current_coord.y;//cm
    return sqrt(pow(dist_vect.i, 2) + pow(dist_vect.j, 2));//cm
}

void reset_origin()
{
    current_coord.x = 0;
    current_coord.y = 0;
    current_angle = 90;
}

int8_t angle_to_target(float* angle)
{
    vector_2D dist_vect;
    vector_2D dir_vect;
    dist_vect.i = target_coord.x - current_coord.x;
    dist_vect.j = target_coord.y - current_coord.y;
    dir_vect.i = cos(TO_RAD(current_angle));
    dir_vect.j = sin(TO_RAD(current_angle));
    float dot_product = dir_vect.i*dist_vect.i + dir_vect.j*dist_vect.j;
    float k = dir_vect.i*dist_vect.j - dir_vect.j*dist_vect.i;//k-component of cross product
    float target_dist = sqrt(pow(dist_vect.i, 2) + pow(dist_vect.j, 2));

    //check for division by zero
    *angle = (target_dist > 0.01) ? TO_DEG(acos(dot_product / target_dist)) : 0;

    return (k < 0) ? RIGHT : LEFT;
}


void set_speed(uint16_t speed)
{
    spd = speed;
}

void set_target_coord(coord xy)
{
    target_coord = xy;
}

inline void turned_right(float angle)
{
    current_angle -= angle;
}

void turned_left(float angle)
{
    current_angle += angle;
}

void update_position(float mm_dist_moved)
{
    current_coord.x += mm_dist_moved * 0.1 * cos(TO_RAD(current_angle));
    current_coord.y += mm_dist_moved * 0.1 * sin(TO_RAD(current_angle));
}

float turn_left(oi_t* sensor_data, float degrees)
{
    float start_angle = 0;

    //get and discard the first values read by io_update function
    //this ensures that the sensor data is fresh
    oi_update(sensor_data);
    
    oi_setWheels(spd, -spd);
    
    while (start_angle < fabs(degrees))
    {
        oi_update(sensor_data);
        start_angle += sensor_data->angle;
    }

    oi_setWheels(0, 0);
    
    turned_left(start_angle);
    return start_angle;
}

float turn_right(oi_t* sensor_data, float degrees)
{
    float start_angle = 0;
    
    //get and discard the first values read by io_update function
    //this ensures that the sensor data is fresh
    oi_update(sensor_data);

    oi_setWheels(-spd, spd);

    while (start_angle < fabs(degrees))
    {
        oi_update(sensor_data);
        start_angle += fabs(sensor_data->angle);
    }

    oi_setWheels(0, 0);

    turned_right(start_angle);//update global angle variable
    return start_angle;
}

float move_forward(oi_t *sensor_data, float distance_mm, uint8_t (*stop_event)(oi_t*))
{
    float sum = 0;
    
    //get and discard the first values read by io_update function
    //this ensures that the sensor data is fresh
    oi_update(sensor_data);
    
    oi_setWheels(spd,spd);
	
    while (sum < fabs(distance_mm) && !(*stop_event)(sensor_data))
    {
        oi_update(sensor_data);
        sum += sensor_data->distance;
    }
    //stop robot
    oi_setWheels(0,0);

    update_position(sum);//update global position variable
    return sum;
}

void turn_to_target_coord(oi_t *sensor_data)
{
    //turn towards target
    float target_angle;
    switch (angle_to_target(&target_angle))
    {
    case RIGHT:
        turn_right(sensor_data, target_angle);
        break;
    case LEFT:
        turn_left(sensor_data, target_angle);
        break;
    }
}

uint8_t move_to_target_coord(oi_t *sensor_data, float padding, uint8_t (*stop_event)(oi_t*))
{
    float move_dist;
    float dist_moved;

    //turn towards target
    turn_to_target_coord(sensor_data);

    //move to target
    move_dist = 10*dist_to_target() - padding;
    dist_moved = move_forward(sensor_data, move_dist, stop_event);

    //return 1 if stopped
    if ((move_dist - dist_moved) > 0.1) return 1;
    else return 0;
}

float move_backward(oi_t *sensor_data, float distance_mm)
{
    float sum = 0;
    
    //get and discard the first values read by io_update function
    //this ensures that the sensor data is fresh
    oi_update(sensor_data);
    
    oi_setWheels(-spd,-spd);
    
    while (sum < fabs(distance_mm))
    {
        oi_update(sensor_data);
        sum += fabs(sensor_data->distance);
    }
    //stop robot
    oi_setWheels(0,0);

    update_position(-sum);//update global position variable
    return sum;
}
