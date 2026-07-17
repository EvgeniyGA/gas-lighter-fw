/*
 * sram_driver.h
 *
 *  Created on: Jul 17, 2026
 *      Author: evgeny
 */

#ifndef INC_SRAM_DRIVER_H_
#define INC_SRAM_DRIVER_H_

#include <stdint.h>
#include <stdio.h>

uint8_t memory_write(void* dest, void* src, size_t BufferSize);
uint8_t memory_read(void* dest, void* src, size_t BufferSize);

#endif /* INC_SRAM_DRIVER_H_ */
