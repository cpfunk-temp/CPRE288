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

//struct to store collected sensor data and angles

void putty_puts(const char* str)
{
    while (*str != '\0')
    {
        cyBot_sendByte(*str);
        str++;
    }
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

//uses the ping sensor right and left edges as the min and max range to search for an object
//note: Currently this function can only detect 1 object
obj_data ir_findObj(sensor_data* data_buffer, uint8_t buffer_size)
{
	obj_data object;
	
	//find largest and smallest ir values and difference between them
	float maxVal = findMax(data_buffer, buffer_size);
	float minVal = findMin(data_buffer, buffer_size);
	float valDelta = maxVal - minVal;
	
	printf("%.2f %.2f %.2f\n", maxVal, minVal, valDelta);
	
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
		if (!right_edge && data_buffer[i].ir_val > 0.5) //positive edge of object
		{
			right_edge = data_buffer[i].angle;
		}

		if (right_edge)
		{
			width++;
		}

		if (right_edge && data_buffer[i].ir_val < 0.5) //negative edge of object
		{
			//to count as an object, the object must have a minimum of 5 itterations over 0.5
			if (width > 5)
			{
                left_edge = data_buffer[i].angle;

                object.delta = abs(left_edge - right_edge);
                object.bisector = right_edge + object.delta/2;
                object.dist = data_buffer[buffer_size/2].dist;
                object.width = obj_width(object.delta, object.dist);

                return object; //return first object found
			}
			else
			{
				right_edge = 0;
				width = 0;
			}
		}
	}
	
	//set all values to zero or EOF if no object found
	object.delta = -1;
	object.bisector = -1;
	object.dist = -1;
	object.width = -1;
	
	return object; 
}

sensor_data* add_dataPoint(sensor_data* data_buffer, uint8_t* const data_buffer_sz, const cyBOT_Scan_t* const scan, uint8_t angle)
{
	(*data_buffer_sz)++;
	uint8_t alloc_sz =  8 * (1 + *data_buffer_sz / 8) ;
	if (*data_buffer_sz % 8 == 1) //allocate memory in chunks of 8 elements
	{		
		data_buffer = realloc(data_buffer, alloc_sz * sizeof(sensor_data));
	}
	data_buffer[*data_buffer_sz - 1].ir_val = (float)scan->IR_raw_val;
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

void free_objs(obj_data** objects, uint8_t* const size)
{
	free(*objects);
	*objects = NULL;
	*size = 0;
}

obj_data* getObjects(obj_data* obj_list, uint8_t* const obj_list_sz, short cm_ping_cutoff)
{
    uint8_t objCnt = 0;
    char txt_buffer[128];
    cyBOT_Scan_t scan;
	obj_data obj;
    //array to txt_buffer sensor data and angles
	uint8_t data_buffer_sz = 0; //ALWAYS initialize data_buffer_sz to zero, it will be resized by add_dataPoint
    sensor_data* data_buffer = NULL; //ALWAYS initialize data_buffer to NULL (otherwise realloc() won't work)

    //if you remove this part, the field will be "scanned" twice
    cyBOT_Scan(0, NULL);
    //wait for the servo to move to zero
    timer_waitMillis(500);
	
    //initial output formatting
    lcd_printf(""); //clear lcd

    //start scaning 180 degrees
    uint8_t angle;
	short right_edge = 0;
    for (angle = 0; angle <= 180; angle += 2)
    {
		//get sensor data
        cyBOT_Scan(angle, &scan);

        //check for ping distance below cutoff
        if (!right_edge && scan.sound_dist < cm_ping_cutoff) //right edge detection
        {
            right_edge = angle;
        }
		
		//start buffering sensor data after right edge is detected
		if (right_edge)
		{
			//add_sensorDataVal allocates memory and adds current sensor values to data_buffer array
			data_buffer = add_dataPoint(data_buffer, &data_buffer_sz, &scan, angle);
			//make sure the memory for the new values was successfully allocated
			if (data_buffer == NULL) lcd_printf("allocation for sensor value buffer failed");
		}
		
        if (right_edge && scan.sound_dist > cm_ping_cutoff) //left edge detection
        {
			//object must have a minimum "width" of 5 data values (10 degrees for increments of 2 degrees)
			//this condition shoud help filter out spikes in sensor data
			if (data_buffer_sz > 5) // if both left and right edges are detected, find object width
			{	
			    //disregard data in data_buffer if the values include 0 or 180
			    if (data_buffer[0].angle != 0 && data_buffer[data_buffer_sz - 1].angle != 180)
			    {
	                //pass data_buffer to ir filter
	                obj = ir_findObj(data_buffer, data_buffer_sz);

	                if (obj.delta != -1)
	                {
	                    objCnt++;
	                    obj_list = add_obj(obj_list, obj_list_sz, &obj);
	                }
	                else
	                {
	                    lcd_printf("no object found by ir filter");
	                }

	                //output for debugging purposes only                                                                //FIXME for debugging only
	                //print collected ir and ping data
	                uint8_t j;
	                for(j = 0; j < data_buffer_sz; j++)
	                {
	                    snprintf(txt_buffer, 200, "%d %.2f %.2f\n\r", data_buffer[j].angle, data_buffer[j].dist, data_buffer[j].ir_val);
	                    putty_puts(txt_buffer);
	                }
	                lcd_printf("%.2f, %d, %d, %.2f", obj.width, obj.delta, obj.bisector, obj.dist);
	                snprintf(txt_buffer, 200, "%.2f, %d, %d, %.2f\n\r", obj.width, obj.delta, obj.bisector, obj.dist); //FIXME
	                putty_puts(txt_buffer);

			    }
				//release data_buffer resources
				free_dataPoints(&data_buffer, &data_buffer_sz);
				
				//reset right_edge to zero
				right_edge = 0;
			}
			else //object detected was too thin
			{
				//release data_buffer resources
				free_dataPoints(&data_buffer, &data_buffer_sz);
				
				//reset right_edge to zero
				right_edge = 0;
			}
        }
    }
	
	//make sure values in data_buffer are freed
	free_dataPoints(&data_buffer, &data_buffer_sz);

    //print the number of objects found								//FIXME for debugging only
    lcd_printf("%d", objCnt);
    snprintf(txt_buffer, 200, "\n\r%d\n\r", objCnt);
    putty_puts(txt_buffer);
	
	//return pointer to list of objects found
	return obj_list;
}
