
#ifndef _CONFIG_SERVICE_H
#define _CONFIG_SERVICE_H

#ifdef __cplusplus
 extern "C" {
#endif

#include <stdint.h>
#include "FreeRTOS.h"
#include "semphr.h"

uint8_t config_service_init(void);
SemaphoreHandle_t storage_get_fs_mutex(void);

uint8_t config_save(char* name, uint8_t* data, uint8_t element_size, uint8_t element_count);
uint8_t config_save_float(char* name, uint8_t* data, uint8_t element_size, uint8_t element_count);
uint8_t config_load(char* name, uint8_t* data, uint8_t element_size, uint8_t element_count);
uint8_t config_load_float(char* name, float* data, uint8_t element_size, uint8_t element_count);

#ifdef __cplusplus
 }
#endif

#endif