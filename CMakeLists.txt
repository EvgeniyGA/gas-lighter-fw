cmake_minimum_required(VERSION 3.22)

project(gas-lighter-fw LANGUAGES C CXX ASM)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON) 

set(MAIN_EXECUTABLE "gas-lighter-fw")
add_executable(${MAIN_EXECUTABLE})

target_sources(${MAIN_EXECUTABLE} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/freertos_support/freertos.c
    ${CMAKE_CURRENT_SOURCE_DIR}/freertos_support/freertos_hook.c
    ${CMAKE_CURRENT_SOURCE_DIR}/modules/FreeRTOS-Plus-CLI/FreeRTOS_CLI.c
    ${CMAKE_CURRENT_SOURCE_DIR}/modules/tinyusb/src/portable/synopsys/dwc2/dcd_dwc2.c
    ${CMAKE_CURRENT_SOURCE_DIR}/modules/tinyusb/src/portable/synopsys/dwc2/dwc2_common.c
)

target_include_directories(${MAIN_EXECUTABLE} PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/app/inc"
    "${CMAKE_CURRENT_SOURCE_DIR}/modules/FreeRTOS-Plus-CLI"
    "${CMAKE_CURRENT_SOURCE_DIR}/modules/tinyusb/src"
    "${CMAKE_CURRENT_SOURCE_DIR}/modules/tinyusb/hw"
)

set(TARGET_CMAKE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/targets/${TARGET}/cmake/stm32cubemx")

add_library(freertos_config INTERFACE)
target_include_directories(freertos_config SYSTEM INTERFACE
    ${CMAKE_SOURCE_DIR}/freertos_support
)

set( FREERTOS_HEAP "4" CACHE STRING "" FORCE)
set( FREERTOS_PORT GCC_ARM_CM4F)

add_library(fatfs_config INTERFACE)
target_include_directories(fatfs_config SYSTEM INTERFACE
    ${CMAKE_SOURCE_DIR}/support/fatfs
    ${CMAKE_SOURCE_DIR}/support/fatfs/drivers
)

target_link_libraries(fatfs_config INTERFACE freertos_kernel)
#target_link_libraries(fatfs_config INTERFACE STM32_Drivers)

add_subdirectory(${CMAKE_SOURCE_DIR}/support/fatfs)
add_subdirectory(${CMAKE_SOURCE_DIR}/RTT)
add_subdirectory(${CMAKE_SOURCE_DIR}/app)
add_subdirectory(${CMAKE_SOURCE_DIR}/modules/freertos)
add_subdirectory(${CMAKE_SOURCE_DIR}/modules/tinyusb/src)
add_subdirectory("${TARGET_CMAKE_DIR}" "${CMAKE_BINARY_DIR}/stm32cubemx_${TARGET}")

tinyusb_target_add(${MAIN_EXECUTABLE})

target_link_libraries(${MAIN_EXECUTABLE} 
    freertos_kernel
    fatfs_core
)
