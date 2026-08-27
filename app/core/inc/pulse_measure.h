/*
 * pulse_measure.c
 *
 *  Created on: Aug 11, 2026
 *      Author: evgeny
 */

#ifndef INC_PULSE_MEASURE_C_
#define INC_PULSE_MEASURE_C_

#include <stdint.h>
#include "app.h"

typedef enum {
	PULSE_MEASURE_OFFSET_ZERO = 0,
	PULSE_MEASURE_OFFSET_HALF
}pulse_measure_offset_e;

typedef struct{
	uint8_t adc_num;
	uint16_t* buf_adc_in;
	uint16_t buf_adc_in_size;
	uint16_t real_measure_count;//todo
}pulseMeasureConfig_s;

void pulse_measure_adc_callback(uint8_t offset);
void pulse_measure_init(pulseMeasureConfig_s* pulse_measure_config);

#endif /* INC_PULSE_MEASURE_C_ */
