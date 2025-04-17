#include <stdlib.h>
#include <math.h>
#include <stdio.h>
#include <timer.h>
#include <string.h>
#include "lcd.h"
#include "button.h"
#include "servo.h"


int main(void)
{
    servo_init();
    set_servo_calibration(37560, 8784);
    turn_servo_to(90);

    //cyBOT_SERVO_cal();
    return 0;
}
