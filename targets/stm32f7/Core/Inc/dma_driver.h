#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"
#include <stdint.h>

void dma_send_data_to_fsmc(int* buf, uint32_t size);

#ifdef __cplusplus
}
#endif
