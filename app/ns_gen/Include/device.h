#pragma once

#include <stdint.h>

#define MAIN_FREQENCY_HZ				(10000)

#ifdef __cplusplus
extern "C" {
#endif

void heartbit_callback(void);
void init_device(void);
uint8_t load_configs(void);

#ifdef __cplusplus
}
#endif
