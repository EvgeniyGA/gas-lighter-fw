#include "fatfs.h"

char sramMas[TOTAL_DISK_SIZE];

char sramPath[4];
FATFS USERFatFS;
FIL file;
SemaphoreHandle_t xDiskMutex = NULL;

void FATFS_Init(void){
    static BYTE work[512] = {0};
    xDiskMutex = xSemaphoreCreateMutex();
	if (xDiskMutex == NULL) {
		while(1);
	}

	if (FATFS_LinkDriver( &SRAMDISK_Driver, sramPath) != 0) {
		printf("Cannot link Disk driver\n");
		//vTaskDelay(1000 / portTICK_PERIOD_MS);
	}

	MKFS_PARM fmt_opt = {FM_SFD | FM_ANY, 0, 0, 0, 0};
	FRESULT fr = f_mkfs(sramPath, &fmt_opt, work, sizeof(work));

	FRESULT res = f_mount(&USERFatFS, sramPath, 1);
	if (res != FR_OK){
		printf("Cannot mount FS!\n");
		//vTaskDelay(1000 / portTICK_PERIOD_MS);
	}

	fr = f_open(&file, "0:/README.TXT", FA_WRITE | FA_CREATE_ALWAYS);
	if (fr == FR_OK) {
		char data[] = "Hello from SRAM disk!";
		UINT bw;
		f_write(&file, data, sizeof(data), &bw);
		f_close(&file);
		printf("FatFS ready to use\n");
	}
}