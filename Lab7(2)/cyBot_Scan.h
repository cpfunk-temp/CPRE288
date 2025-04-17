#ifndef CYBOT_SCAN_H_
#define CYBOT_SCAN_H_

#include <stdint.h>
#include "adc.h"
#include "ping.h"
#include "servo.h"

// Scan value
typedef struct{
    float sound_dist;  // Distance from PING sensor
    uint16_t IR_raw_val;    // Raw ADC value from IR sensor
} cyBOT_Scan_t;



void cyBOT_init_Scan(uint8_t ir_oversampling);

// Point sensors to angle, and get a scan value
void cyBOT_Scan(int16_t angle, cyBOT_Scan_t* getScan);

#endif //!CYBOT_SCAN_H_
