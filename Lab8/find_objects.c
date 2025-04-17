#include <stdlib.h>
#include <math.h>
#include <stdio.h>
#include <timer.h>
#include <string.h>
#include "open_interface.h"
#include "lcd.h"
#include "move.h"
#include "cyBot_uart.h"
#include "cyBot_Scan.h"
#include "find_objects.h"
#include "uart-interrupt.h"
#include "adc.h"

obj_data findSmallestObj(obj_data* objects, uint8_t obj_count)
{
    obj_data min = *objects;
    uint8_t i;
    for (i = 0; i < obj_count; i++)
    {
        if (objects[i].width < min.width)
        {
            min = objects[i];
        }
    }

    return min;
}

float obj_width(short delta_angle, float ping_dist)
{
    return fabs(2*ping_dist*tan(abs(delta_angle)*M_PI/360.0f));
}

float findMax(sensor_data* ir_vals, uint8_t size)
{
	float maxVal = ir_vals->ir_val;
	ir_vals++;
	
	uint8_t i;
	for (i = 1; i < size; i++)
	{
		if (ir_vals->ir_val > maxVal)
		{
			maxVal = ir_vals->ir_val;
		}
		ir_vals++;
	}
	return maxVal;
}

float findMin(sensor_data* ir_vals, uint8_t size)
{
	float minVal = ir_vals->ir_val;
	ir_vals++;
	
	uint8_t i;
	for (i = 1; i < size; i++)
	{
		if (ir_vals->ir_val < minVal)
		{
			minVal = ir_vals->ir_val;
		}
		ir_vals++;
	}
	return minVal;
}

obj_data* add_obj(obj_data* objects, uint8_t* const size, obj_data* new_obj)
{
    (*size)++;
    uint8_t alloc_sz = 4 * (1 + *size / 4);
    if (*size % 4 == 1) //allocate memory in chunks of 4 elements
    {
        objects = realloc(objects, alloc_sz * sizeof(obj_data));
    }
    objects[*size - 1] = *new_obj;

    return objects;
}

//uses the ping sensor right and left edges as the min and max range to search for an object
//note: Currently this function can only detect 1 object
obj_data* ir_findObj(sensor_data* data_buffer, uint8_t buffer_size, obj_data* obj_list, uint8_t* obj_list_sz)
{
    obj_data obj;
	
	//find largest and smallest ir values and difference between them
	float maxVal = findMax(data_buffer, buffer_size);
	float minVal = findMin(data_buffer, buffer_size);
	float valDelta = maxVal - minVal;
	
	//convert all numbers into values between 1.0 and 0.0 and square all values
	uint8_t i;
	for(i = 0; i < buffer_size; i++)
	{
		data_buffer[i].ir_val -= minVal;
		data_buffer[i].ir_val /= valDelta;
		data_buffer[i].ir_val = pow(data_buffer[i].ir_val, 2);
	}
	//loop through ir data and check for object
	uint8_t width = 0;
	short right_edge = 0;
	short left_edge = 0;
	for (i = 0; i < buffer_size; i++)
	{
		//start counting the width of an object when a value goes over 0.5
		if (!right_edge && data_buffer[i].ir_val > 0.2) //positive edge of object
		{
			right_edge = data_buffer[i].angle;
		}

		if (right_edge)
		{
			width++;
		}

		if (right_edge && data_buffer[i].ir_val < 0.2) //negative edge of object
		{
			//to count as an object, the object must have a minimum of 5 itterations over 0.5
			if (width > 3)
			{
                left_edge = data_buffer[i].angle;

                obj.delta = abs(left_edge - right_edge);
                obj.bisector = right_edge + obj.delta/2;
                obj.dist = data_buffer[buffer_size/2].dist;
                obj.width = obj_width(obj.delta, obj.dist);

                obj_list = add_obj(obj_list, obj_list_sz, &obj);

                //reset right_edge to zero
                right_edge = 0;
                //reset width to zero
                width = 0;
			}
			else
			{
			    //reset right_edge to zero
				right_edge = 0;
				//reset width to zero
				width = 0;
			}
		}
	}

	return obj_list;
}

sensor_data* add_dataPoint(sensor_data* data_buffer, uint8_t* const data_buffer_sz, const cyBOT_Scan_t* const scan, uint16_t ir_data, uint8_t angle)
{
	(*data_buffer_sz)++;
	uint8_t alloc_sz =  8 * (1 + *data_buffer_sz / 8) ;
	if (*data_buffer_sz % 8 == 1) //allocate memory in chunks of 8 elements
	{		
		data_buffer = realloc(data_buffer, alloc_sz * sizeof(sensor_data));
	}
	data_buffer[*data_buffer_sz - 1].ir_val = (float)ir_data;
	data_buffer[*data_buffer_sz - 1].dist = scan->sound_dist;
	data_buffer[*data_buffer_sz - 1].angle = angle;
	
	return data_buffer;
}

void free_dataPoints(sensor_data** data_buffer, uint8_t* const data_buffer_sz)
{
	free(*data_buffer);
	*data_buffer = NULL;
	*data_buffer_sz = 0;
}

void free_objs(obj_data** objects, uint8_t* const size)
{
	free(*objects);
	*objects = NULL;
	*size = 0;
}

float moving_avg(float avg, uint16_t cnt, float data)
{
    return (cnt != 0) ? avg + (data - avg) / cnt : 0;
}

obj_data* getObjects(uint8_t* const distant_obj_detected, float* const distant_obj_angle, float* const distant_obj_dist, obj_data* obj_list, uint8_t* const obj_list_sz, short cm_ping_range)
{
    cyBOT_Scan_t scan;
    //array to txt_buffer sensor data and angles
	uint8_t data_buffer_sz = 0; //ALWAYS initialize data_buffer_sz to zero, it will be resized by add_dataPoint
    sensor_data* data_buffer = NULL; //ALWAYS initialize data_buffer to NULL (otherwise realloc() won't work)

    //if you remove this part, the field will be "scanned" twice
    cyBOT_Scan(0, NULL);
    //wait for the servo to move to zero
    timer_waitMillis(500);

    //start scaning 180 degrees
    uint8_t angle;
    uint16_t ir_data;
	short right_edge = 0;
	float obj_angle;
	*distant_obj_detected = 0;
	*distant_obj_angle = 0;
	*distant_obj_dist = 0;
    for (angle = 0; angle <= 180; angle += 2)
    {
		//get sensor data
        cyBOT_Scan(angle, &scan);
        ir_data = adc_read();

        //check for ping distance below cutoff
        if (!right_edge && scan.sound_dist < cm_ping_range) //right edge detection
        {
            right_edge = angle;
        }
		
		//start buffering sensor data after right edge is detected
		if (right_edge)
		{
			//add_sensorDataVal allocates memory and adds current sensor values to data_buffer array
			data_buffer = add_dataPoint(data_buffer, &data_buffer_sz, &scan, ir_data, angle);

			//make sure the memory for the new values was successfully allocated
			if (data_buffer == NULL) lcd_printf("allocation for sensor value buffer failed");
		}
		
        if (right_edge && scan.sound_dist > cm_ping_range) //left edge detection
        {
            uint8_t old_obj_list_sz = *obj_list_sz;
            //pass data_buffer to ir filter
            obj_list = ir_findObj(data_buffer, data_buffer_sz, obj_list, obj_list_sz);

            //if ir_findObj didn't find objects, find average angle to all objects
            if (*distant_obj_detected || *obj_list_sz - old_obj_list_sz == 0)
            {
                ++(*distant_obj_detected);
            }
            else
            {
                *distant_obj_detected = 0;
            }
            obj_angle = (angle + right_edge) * 0.5;
            *distant_obj_angle = moving_avg(*distant_obj_angle, *distant_obj_detected, obj_angle);
            *distant_obj_dist = moving_avg(*distant_obj_dist, *distant_obj_detected, data_buffer[data_buffer_sz / 2 - 1].dist);

            //release data_buffer resources
            free_dataPoints(&data_buffer, &data_buffer_sz);

            //reset right_edge to zero
            right_edge = 0;
        }
    }
	
	//make sure values in data_buffer are freed
	free_dataPoints(&data_buffer, &data_buffer_sz);
	
	//return pointer to list of objects found
	return obj_list;
}
