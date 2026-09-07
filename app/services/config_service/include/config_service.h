
#ifndef _CONFIG_SERVICE_H
#define _CONFIG_SERVICE_H

#include <stdint.h>
#include "FreeRTOS.h"
#include "semphr.h"

uint8_t config_service_init(void);
SemaphoreHandle_t storage_get_fs_mutex(void);
uint8_t config_read(char* name, uint8_t* data, uint16_t datalen);

#endif