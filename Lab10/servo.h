#ifndef SERVO_H_
#define SERVO_H_

void set_servo_calibration(uint32_t left_val, uint32_t right_val);

//initialize the servo
void servo_init();

void turn_servo_to(int16_t degrees);

//function to calibrate the servo
void cyBOT_SERVO_cal();

#endif //!SERVO_H_
