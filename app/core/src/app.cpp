#include "app.h"
#include "main.h"
#include <stdio.h>
#include <ctype.h>
#include "SEGGER_RTT.h"
#include "SEGGER_SYSVIEW.h"
#include "fatfs.h"
#include "version.h"
#include "version_check.h"
#include "cli_service.h"
#include "status_service.h"
#include "usb_service.h"
#include <FreeRTOS.h>
#include "task.h"
#include "device.h"
#include <array>
#include <cstdio>
#include "config_wrapper.hpp"
#include <type_traits>

usb_device_config_t usb_device_config = {
	.mounted = [](){ std::printf("USB device mounted\n\r"); },
	.unmounted = [](){ std::printf("USB unmounted"); }
};

uint8_t load_configs(void);
void print_configs(void);

void init(void){
#ifndef FOR_QEMU
	SEGGER_RTT_ConfigUpBuffer( 0, NULL, NULL, 0, SEGGER_RTT_MODE_NO_BLOCK_TRIM );
#endif
	SEGGER_SYSVIEW_Conf();
  	SEGGER_SYSVIEW_Start();
  	while(SEGGER_SYSVIEW_IsStarted()==0);
  	SEGGER_RTT_WriteString( 0, "SEGGER Real-Time-Terminal Started\n" );
}

void setup(void){
	printf("Firmware version: %s\n", FW_VERSION_STR);
	printf("Build: %s %s (git: %s)\n", FW_BUILD_DATE, FW_BUILD_TIME, FW_GIT_HASH);
	printf("Version: %d.%d.%d\n", FW_VERSION_MAJOR, FW_VERSION_MINOR, FW_VERSION_PATCH);

	if (is_hash_invalid(FW_GIT_HASH)) {
		printf("ERROR: Invalid firmware hash detected: %s\r\n", FW_GIT_HASH ? FW_GIT_HASH : "NULL");
	} else {
		printf("FW Hash: %s\r\n", FW_GIT_HASH);
	}
	
	FATFS_Init();
	xTaskCreate([](void* param){
		cli_service_init();
		config_service_init();
		status_service_init(device::heartbit_callback);
		usb_device_init(&usb_device_config);
		usb_cdc_init();
		load_configs();
		print_configs();
		device::init();
		vTaskDelete(NULL);
	}, "init", configMINIMAL_STACK_SIZE * 2, NULL, 1, NULL);

	
	vTaskStartScheduler();
}

uint8_t load_configs(void){
	std::printf("Load configs ...\n\r");
	device::config.apply([](auto& item){
		if(item.load() == true){
			std::printf("\t[%s] loaded\n\r", item.name);
		}
		else{
			std::printf("\t[%s] use default\n\r", item.name);
		}
	});
    return 0;
}

template <typename T>
struct is_std_array : std::false_type {};

template <typename T, std::size_t N>
struct is_std_array<std::array<T, N>> : std::true_type {};

template <typename T>
inline constexpr bool is_std_array_v = is_std_array<T>::value;

void print_configs(void){
	std::printf("Print configs...\n\r");
	device::config.apply([](auto& item){
		using UsedType = std::decay_t<decltype(item.value)>;
		if constexpr (std::is_same_v<UsedType, uint32_t>){
			std::printf("\t[%s] = %d\n\r", item.name, static_cast<int>(item.value));
		}
		else if constexpr (std::is_same_v<UsedType, float>){
			std::printf("\t[%s] = %.3f\n\r", item.name, item.value);
		}
		else if constexpr (is_std_array_v<UsedType>){
			std::printf("\t [%s] =\n\r", item.name);
			for(const auto& i: item.value){
				std::printf("\t\t%d\n\r", i);
			}
		}
		else{
			std::printf("\tError! [%s] - unsupported type", item.name);
		}
	});
}

