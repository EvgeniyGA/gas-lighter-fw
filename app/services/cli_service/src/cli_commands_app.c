#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include "fatfs.h"
#include "FreeRTOS.h"
#include "FreeRTOS_CLI.h"

static BaseType_t prvSetADC_PeriodDivider( char *pcWriteBuffer,
                                  size_t xWriteBufferLen,
                                  const char *pcCommandString )
{
	uint32_t divider;
	BaseType_t xParameter1StringLength;

    divider = atoi(FreeRTOS_CLIGetParameter(pcCommandString, (UBaseType_t)1, (BaseType_t*)&xParameter1StringLength));

	if (divider > 0){
		if(puse_measure_set_divider(divider) == 0){
			sprintf(pcWriteBuffer, "adc period divider %ld setted\r\n", divider);
		}
		else{
			sprintf(pcWriteBuffer, "ERROR! adc period divider wrong value\r\n");
		}
	}else {
		sprintf(pcWriteBuffer, "ERROR! adc period divider must be greater then zero\r\n");
	}

	return pdFALSE;
}

static BaseType_t prvSetDAC_TimerDivider( char *pcWriteBuffer,
                                  size_t xWriteBufferLen,
                                  const char *pcCommandString )
{
	uint32_t divider;
	BaseType_t xParameter1StringLength;

    divider = atoi(FreeRTOS_CLIGetParameter(pcCommandString, (UBaseType_t)1, (BaseType_t*)&xParameter1StringLength));

	if ((divider > 0) && (divider <= 10000)){
		if(wave_starter_set_divider(divider) == 0){
			sprintf(pcWriteBuffer, "adc period divider %ld setted\r\n", divider);
		}
	}else {
		sprintf(pcWriteBuffer, "ERROR! adc period divider wrong\r\n");
	}

	return pdFALSE;
}

const CLI_Command_Definition_t xSetADC_PeriodDividerCommand =
{
    "meandr_div",
	"meandr_div: Set Meandr Out Freqency Divider\r\n",
	prvSetADC_PeriodDivider,
    1
};

const CLI_Command_Definition_t xSetDAC_TimerDividerCommand =
{
    "sin_div",
	"sin_div: Set Sinus Freqency Divider\r\n",
	prvSetDAC_TimerDivider,
    1
};

BaseType_t CLI_install_commands_app(void){
	FreeRTOS_CLIRegisterCommand(&xSetADC_PeriodDividerCommand);
	FreeRTOS_CLIRegisterCommand(&xSetDAC_TimerDividerCommand);

	return pdPASS;
}