#ifndef MOVE_H_
#define MOVE_H_


typedef struct
{
    float x;
    float y;
} coord;

void set_speed(uint16_t speed);

void set_target_coord(coord xy);

void reset_origin();

float turn_right(oi_t* sensor_data, float degrees);

float turn_left(oi_t* sensor_data, float degrees);

float move_forward(oi_t *sensor_data, float distance_mm, uint8_t (*stop_event)(oi_t*));

uint8_t move_to_target_coord(oi_t *sensor_data, float padding, uint8_t (*stop_event)(oi_t*));

void turn_to_target_coord(oi_t *sensor_data);

float move_backward(oi_t *sensor_data, float distance_mm);

#endif // !MOVE_H_
