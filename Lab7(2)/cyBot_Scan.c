#include "cyBot_Scan.h"

//the hardware takes 2^ir_oversampling samples and averages them
void cyBOT_init_Scan(uint8_t ir_oversampling)
{
    adc_init(ir_oversampling);
    ping_init();
    servo_init();
}

void cyBOT_Scan(int16_t angle, cyBOT_Scan_t* getScan)
{
    turn_servo_to(angle);
    getScan->sound_dist = get_ping_dist();
    getScan->IR_raw_val = adc_read();
}
