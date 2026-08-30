/*
 * dac_driver.h
 *
 *  Created on: Mar 31, 2026
 *      Author: evgeny
 */

#ifndef INC_DAC_DRIVER_H_
#define INC_DAC_DRIVER_H_

#include <stdint.h>

int dac_start(uint16_t* buf, uint32_t size, uint16_t tim_arr);
void dac_timer_set_prescaler(uint32_t presc);

#endif /* INC_DAC_DRIVER_H_ */
