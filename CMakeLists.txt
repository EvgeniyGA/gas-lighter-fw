cmake_minimum_required(VERSION 3.22)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

file(READ "${CMAKE_SOURCE_DIR}/VERSION" FW_VERSION_RAW)
string(STRIP "${FW_VERSION_RAW}" FW_VERSION)

project(gas-lighter-fw VERSION ${FW_VERSION} LANGUAGES C CXX ASM)

include(version)
generate_version_header()

if(FOR_QEMU)
    add_compile_definitions(MSC_USE_SRAM)
    add_compile_definitions(PRINTF_UART)
else()
    add_compile_definitions(MSC_USE_FLASH)
    add_compile_definitions(PRINTF_RTT)
endif()

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON) 

if(NOT DEFINED TARGET)
#    set(TARGET "stm32f4" CACHE STRING "Target hardware platform")
    message(FATAL_ERROR "TARGET not specified: ${TARGET}")
endif()

set(MAIN_EXECUTABLE "gas-lighter-fw")
add_executable(${MAIN_EXECUTABLE})

target_sources(${MAIN_EXECUTABLE} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/support/freertos_support/freertos.c
    ${CMAKE_CURRENT_SOURCE_DIR}/support/freertos_support/freertos_hook.c
    ${CMAKE_CURRENT_SOURCE_DIR}/modules/FreeRTOS-Plus-CLI/FreeRTOS_CLI.c
    ${CMAKE_CURRENT_SOURCE_DIR}/support/tinyusb/msc_disk.c
    ${CMAKE_CURRENT_SOURCE_DIR}/support/tinyusb/usb_descriptors.c
    ${CMAKE_CURRENT_SOURCE_DIR}/modules/tinyusb/src/portable/synopsys/dwc2/dcd_dwc2.c
    ${CMAKE_CURRENT_SOURCE_DIR}/modules/tinyusb/src/portable/synopsys/dwc2/dwc2_common.c
    ${CMAKE_CURRENT_SOURCE_DIR}/targets/${TARGET}/Core/Src/W25Qxx.c
    ${CMAKE_CURRENT_SOURCE_DIR}/targets/${TARGET}/Core/Src/adc_driver.c
    ${CMAKE_CURRENT_SOURCE_DIR}/targets/${TARGET}/Core/Src/dac_driver.c
)

target_include_directories(${MAIN_EXECUTABLE} PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/app/inc"
    "${CMAKE_CURRENT_SOURCE_DIR}/support/tinyusb"
    "${CMAKE_CURRENT_SOURCE_DIR}/modules/FreeRTOS-Plus-CLI"
    "${CMAKE_CURRENT_SOURCE_DIR}/modules/tinyusb/src"
    "${CMAKE_CURRENT_SOURCE_DIR}/modules/tinyusb/hw"
    "${CMAKE_BINARY_DIR}/generated"
)

set(TARGET_CMAKE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/targets/${TARGET}/cmake/stm32cubemx")

add_library(freertos_config INTERFACE)
target_include_directories(freertos_config SYSTEM INTERFACE
    ${CMAKE_SOURCE_DIR}/support/freertos_support
)

target_link_libraries(freertos_config INTERFACE segger_sysview)

set( FREERTOS_HEAP "4" CACHE STRING "" FORCE)
if(${TARGET} STREQUAL "stm32f4")
    set( FREERTOS_PORT GCC_ARM_CM4F)
elseif(${TARGET} STREQUAL "stm32f7")
    set( FREERTOS_PORT GCC_ARM_CM7)
else()
    message(FATAL_ERROR "Неизвестный TARGET: '${TARGET}'. Добавьте настройку FREERTOS_PORT для этой платформы.")
endif()

add_library(fatfs_config INTERFACE)
target_include_directories(fatfs_config SYSTEM INTERFACE
    ${CMAKE_SOURCE_DIR}/support/fatfs
    ${CMAKE_SOURCE_DIR}/support/fatfs/drivers
)

target_link_libraries(fatfs_config INTERFACE freertos_kernel STM32_Drivers)

add_subdirectory(app)
add_subdirectory(support/fatfs)
add_subdirectory(support/RTT)
add_subdirectory(support/SystemView)
add_subdirectory(modules/freertos)
add_subdirectory(modules/tinyusb/src)
add_subdirectory(modules/CMSIS-DSP)
add_subdirectory("${TARGET_CMAKE_DIR}" "${CMAKE_BINARY_DIR}/stm32cubemx_${TARGET}")

tinyusb_target_add(${MAIN_EXECUTABLE})

target_include_directories(CMSISDSP PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}/targets/${TARGET}/Drivers/CMSIS/Include
    ${CMAKE_CURRENT_SOURCE_DIR}/targets/${TARGET}/Drivers/CMSIS/Device/ST/STM32F4xx/Include
)

target_link_libraries(${MAIN_EXECUTABLE} 
    freertos_kernel
    fatfs_core
    CMSISDSP
    segger_rtt         
    segger_sysview 
)
