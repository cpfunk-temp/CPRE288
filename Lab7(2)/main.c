#include <stdlib.h>
#include <math.h>
#include <stdio.h>
#include <timer.h>
#include <string.h>
#include "open_interface.h"
#include "lcd.h"
#include "move.h"
#include "find_objects.h"
#include "uart-interrupt.h"

#define TO_RAD(x) (x*M_PI/180.0f)

uint8_t stop_event(oi_t* sensor_data) { return sensor_data->bumpLeft || sensor_data->bumpRight; }

void on_bump(oi_t* sensor_data)
{
    float moved_dist;
    float remain_dist;

    uint8_t bumpLeft = sensor_data->bumpLeft;
    uint8_t bumpRight = sensor_data->bumpRight;

    if (bumpLeft)
    {
        move_backward(sensor_data, 50);
        turn_right(sensor_data, 90);
        moved_dist = move_forward(sensor_data, 200, stop_event);
        remain_dist = 200.0 - moved_dist;
        if (remain_dist > 0.1)//if the robot was stopped
        {
            on_bump(sensor_data);
        }
        turn_to_target_coord(sensor_data);
    }
    else if (bumpRight)
    {
        move_backward(sensor_data, 50);
        turn_left(sensor_data, 90);
        moved_dist = move_forward(sensor_data, 200, stop_event);
        remain_dist = 200.0 - moved_dist;
        if (remain_dist > 0.1)//if the robot was stopped
        {
            on_bump(sensor_data);
        }
        turn_to_target_coord(sensor_data);
    }
}

void print_objects(obj_data* objs, uint8_t obj_count)
{
    char txt_buffer[200];

    uint8_t i;
    for (i = 0; i < obj_count; i++)
    {
        snprintf(txt_buffer, 200, "Width:%.1f Angle:%d Distance:%.1f Number:%d\r\n", objs[i].width, objs[i].bisector, objs[i].dist, i + 1);
        uart_sendStr(txt_buffer);
    }
}

float avg_obj_dist(obj_data* objs, uint8_t obj_count)
{
    float sum = 0;
    uint8_t i;
    for (i = 0; i < obj_count; i++)
    {
        sum += objs[i].dist;
    }

    return sum / obj_count;
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

coord get_obj_coord(obj_data obj)
{
    coord obj_coord;
    uint8_t cybot_sensor_offset = 4;//cm
    uint8_t dist_to_cybot_center = 10;//cm
    float obj_center_dist = obj.dist + obj.width / 2 + cybot_sensor_offset;
    obj_coord.x = obj_center_dist*cos(TO_RAD(obj.bisector));
    obj_coord.y = dist_to_cybot_center + obj_center_dist*sin(TO_RAD(obj.bisector));
    return obj_coord;//cm
}

coord get_dest_coord(float angle, float dist)
{
    coord obj_coord;
    uint8_t cybot_sensor_offset = 4;//cm
    uint8_t dist_to_cybot_center = 10;//cm
    float obj_center_dist = dist + cybot_sensor_offset;
    obj_coord.x = obj_center_dist*cos(TO_RAD(angle));
    obj_coord.y = dist_to_cybot_center + obj_center_dist*sin(TO_RAD(angle));
    return obj_coord;//cm
}

int main(void)
{
    //initializations
    timer_init();
    lcd_init();
    cyBOT_init_Scan(6);
    cyBOT_Scan(90, NULL);
    uart_interrupt_init();

    //cyBOT_SERVO_cal();
    set_servo_calibration(37560, 8784);
    oi_t *sensor_data = oi_alloc();
    oi_init(sensor_data);
    display_charge(sensor_data);
    uint8_t obj_count = 0;
    obj_data* obj_found = NULL;
    obj_data smallest_obj;
    coord smallest_obj_coord;
    uint8_t failed = 1;
    uint8_t bumped = 0;
    float dist_moved;
    float move_dist;
    float distant_obj_angle;
    float distant_obj_dist;
    uint8_t distant_obj_found;
    coord distant_obj_coord;

    //wait for data to signal start
    uart_receive();

    while (failed)
    {
        //reset origin before scan since get_obj_coord() returns coordinates relative to sensor head
        reset_origin();
        //scan for objects
        obj_found = getObjects(&distant_obj_found, &distant_obj_angle, &distant_obj_dist, obj_found, &obj_count, 100);
        //print objects found
        print_objects(obj_found, obj_count);

        if (obj_count != 0 || distant_obj_found != 0)
        {
            //if object is too far away, move towards object
            if(distant_obj_found)
            {
                lcd_printf("%d object%sseen\nmoving closer...", distant_obj_found, (distant_obj_found == 1) ? " " : "s ");
                timer_waitMillis(500);

                //point sensor head at distant objects
                cyBOT_Scan(distant_obj_angle, NULL);
                timer_waitMillis(500);
                cyBOT_Scan(90, NULL);
                distant_obj_coord = get_dest_coord(distant_obj_angle, distant_obj_dist);
                set_target_coord(distant_obj_coord);
                turn_to_target_coord(sensor_data);
                move_dist = distant_obj_dist * 2.5;
                dist_moved = move_forward(sensor_data, move_dist, stop_event);

                if ((move_dist - dist_moved) > 0.1)
                {
                    on_bump(sensor_data);
                }

                free_objs(&obj_found, &obj_count);
                continue;
            }

            //find smallest object
            smallest_obj = findSmallestObj(obj_found, obj_count);

            //point sensor head at smallest object
            cyBOT_Scan(smallest_obj.bisector, NULL);
            timer_waitMillis(500);
            cyBOT_Scan(90, NULL);

            //set coordinates of smallest object relative to robot
            smallest_obj_coord = get_obj_coord(smallest_obj);
            set_target_coord(smallest_obj_coord);

            if (smallest_obj.dist <= 15)
            {
               failed = 0;
               free_objs(&obj_found, &obj_count);
               continue;
            }

            bumped = move_to_target_coord(sensor_data, 270, stop_event);

            if (bumped)
            {
                on_bump(sensor_data);
            }
            else
            {
                failed = 0;
            }
        }
        else
        {
            lcd_printf("NO OBJECTS");
        }
        
        free_objs(&obj_found, &obj_count);
    }

    lcd_printf("DONE!");

    oi_free(sensor_data);

    return 0;
}
