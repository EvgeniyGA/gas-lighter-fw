
#ifndef _CONFIG_SERVICE_H
#define _CONFIG_SERVICE_H

#ifdef __cplusplus
 extern "C" {
#endif

#include <stdint.h>
#include "FreeRTOS.h"
#include "semphr.h"

typedef enum{
    CONFIG_DATA_TYPE_INT,
    CONFIG_DATA_TYPE_FLOAT
}config_data_type_e;

uint8_t config_service_init(void);
SemaphoreHandle_t storage_get_fs_mutex(void);

uint8_t config_save_raw(const char* name, uint8_t* data, uint8_t element_size, uint8_t element_count, uint8_t type);
uint8_t config_load_raw(const char* name, uint8_t* data, uint8_t element_size, uint8_t element_count, uint8_t type);

#ifdef __cplusplus
 }
#endif

#endif