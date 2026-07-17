/*
 * sram_driver.c
 *
 *  Created on: Jul 17, 2026
 *      Author: evgeny
 */

#include "sram_driver.h"
#include <string.h>

uint8_t memory_write(void* dest, void* src, size_t BufferSize){
	memcpy(dest, src, BufferSize);
	return 0;
}

uint8_t memory_read(void* dest, void* src, size_t BufferSize){
	memcpy(dest, src, BufferSize);
	return 0;
}
