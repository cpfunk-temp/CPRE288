#ifndef MOVE_H_
#define MOVE_H_

void set_speed(uint16_t speed);

float turn_right(oi_t* sensor_data, float degrees);

float turn_left(oi_t* sensor_data, float degrees);

float move_forward (oi_t *sensor_data, float distance_mm);

float move_backward(oi_t *sensor_data, float distance_mm);

#endif // !MOVE_H_
