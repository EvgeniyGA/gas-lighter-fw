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
		sprintf(pcWriteBuffer, "adc period divider %ld setted\r\n", divider);
	}else {
		sprintf(pcWriteBuffer, "ERROR! adc period divider must be greater then zero\r\n");
	}

	return pdFALSE;
}


const CLI_Command_Definition_t xSetADC_PeriodDividerCommand =
{
    "set_div",
	"set_div: Set Out Freqency Divider\r\n",
	prvSetADC_PeriodDivider,
    0
};

BaseType_t CLI_install_commands_app(void){
	FreeRTOS_CLIRegisterCommand(&xSetADC_PeriodDividerCommand);

	return pdPASS;
}