/*
 * pulse_measure.c
 *
 *  Created on: Aug 11, 2026
 *      Author: evgeny
 */

#ifndef INC_PULSE_MEASURE_C_
#define INC_PULSE_MEASURE_C_

#include <stdint.h>
#include "FreeRTOS.h"
#include "queue.h"
//#include "pulse_measure_events.h"

typedef enum{
	Pulse_Measure_ADC_Channel_1 = 0,
	Pulse_Measure_ADC_Channel_2,
    Pulse_Measure_ADC_Channel_3,
    Pulse_Measure_ADC_Channel_4,
	Pulse_Measure_ADC_NumbOfCnannels
}pulse_measure_channels_e;

typedef enum {
	PULSE_MEASURE_OFFSET_ZERO = 0,
	PULSE_MEASURE_OFFSET_HALF
}pulse_measure_offset_e;

typedef struct{
    uint32_t result[Pulse_Measure_ADC_NumbOfCnannels];
}pulse_measure_msg_t;

typedef void(*pulse_measure_data_ready_callback_t)(pulse_measure_msg_t* data);

typedef struct{
	uint8_t adc_num;
	uint16_t* buf_adc_in;
	uint16_t buf_adc_in_size;
	uint16_t real_measure_count;//todo
	pulse_measure_data_ready_callback_t data_ready;

}pulseMeasureConfig_s;

void pulse_measure_adc_callback(uint8_t offset);
void pulse_measure_init(pulseMeasureConfig_s* pulse_measure_config);
uint8_t puse_measure_set_divider(uint32_t divider);

#endif /* INC_PULSE_MEASURE_C_ */
