#ifndef __fatfs_H
#define __fatfs_H
#ifdef __cplusplus
 extern "C" {
#endif

#include <stdio.h>
#include "FreeRTOS.h"
#include "semphr.h"
#include "ff.h"
#include "ff_gen_drv.h"

#include "sram_diskio.h"

extern char sramMas[TOTAL_DISK_SIZE];
extern char sramPath[4];
extern FATFS USERFatFS;
extern FIL file;
extern SemaphoreHandle_t xDiskMutex;

void FATFS_Init(void);

#ifdef __cplusplus
}
#endif
#endif /*__fatfs_H */