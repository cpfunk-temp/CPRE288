/**
 * Driver for ping sensor
 * @file ping.c
 * @author
 */
#ifndef PING_H_
#define PING_H_

#include <stdint.h>
#include <stdbool.h>
#include <inc/tm4c123gh6pm.h>
#include "driverlib/interrupt.h"

/**
 * Initialize ping sensor. Uses PB3 and Timer 3B
 */
void ping_init();

/**
 * @brief Timer3B ping ISR
 */
void TIMER3B_Handler();

/**
 * @brief Calculate the distance in cm
 *
 * @return Distance in cm
 */
float get_ping_dist();

#endif /* PING_H_ */
