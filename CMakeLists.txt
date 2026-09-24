cmake_minimum_required(VERSION 3.22)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

file(READ "${CMAKE_SOURCE_DIR}/VERSION" FW_VERSION_RAW)
string(STRIP "${FW_VERSION_RAW}" FW_VERSION)

project(gas-lighter-fw VERSION ${FW_VERSION} LANGUAGES C CXX ASM)

if(FOR_QEMU)
    add_compile_definitions(MSC_USE_SRAM)
    add_compile_definitions(PRINTF_UART)
    add_compile_definitions(FOR_QEMU)
    set(MAIN_TIME_RESOLUTION 1 CACHE STRING "buffer size")
    set(DISKIO_DRIVER_SRC "drivers/sram_diskio.c")
else()
    add_compile_definitions(MSC_USE_FLASH)
    #add_compile_definitions(MSC_USE_SRAM)
    add_compile_definitions(PRINTF_RTT)
    set(MAIN_TIME_RESOLUTION 32 CACHE STRING "buffer size")
    set(DISKIO_DRIVER_SRC "drivers/flash_diskio.c")
    #set(DISKIO_DRIVER_SRC "drivers/sram_diskio.c")
endif()

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON) 

if(NOT DEFINED TARGET)
    message(FATAL_ERROR "TARGET not specified: ${TARGET}")#    set(TARGET "stm32f4" CACHE STRING "Target hardware platform")
endif()

set(MAIN_EXECUTABLE "gas-lighter-fw")
add_executable(${MAIN_EXECUTABLE})

target_compile_definitions(${MAIN_EXECUTABLE} PRIVATE
    MAIN_TIME_RESOLUTION=${MAIN_TIME_RESOLUTION}
)

include(cmake/version.cmake)
generate_version_header()

#if(${TARGET} STREQUAL "stm32f4")
add_subdirectory(modules/CMSIS-DSP)
#endif()

add_subdirectory(targets)
add_subdirectory(modules)
add_subdirectory(support)
add_subdirectory(app)

if(${TARGET} STREQUAL "stm32f4")
    target_link_libraries(${MAIN_EXECUTABLE} 
        freertos_kernel
        freertos_support
        freertos_cli_core
        cli_service
        fatfs_core
        CMSISDSP
        segger_rtt         
        segger_sysview 
        lcd_service
        wave_starter_service
        wave_measure_service
        pulse_measure_service
        config_service
        status_service
        tinyusb_support
        usb_service
        diskio_drivers
        bsp_drivers
        app_version
    )
elseif(${TARGET} STREQUAL "stm32f7")
    target_link_libraries(${MAIN_EXECUTABLE} 
        freertos_kernel
        freertos_support
        freertos_cli_core
        cli_service
        fatfs_core
        CMSISDSP
        segger_rtt         
        segger_sysview 
        config_service
        status_service
        tinyusb_support
        usb_service
        diskio_drivers
        bsp_drivers
        app_version
    )
else()
    message(FATAL_ERROR "unsupported target ${TARGET}")
endif()


find_program(CMAKE_OBJCOPY NAMES arm-none-eabi-objcopy REQUIRED)

add_custom_command(TARGET ${MAIN_EXECUTABLE} POST_BUILD
    COMMAND ${CMAKE_OBJCOPY} -O ihex $<TARGET_FILE:${MAIN_EXECUTABLE}> ${MAIN_EXECUTABLE}.hex
    BYPRODUCTS ${MAIN_EXECUTABLE}.hex
    COMMENT "Generating Intel HEX file: ${MAIN_EXECUTABLE}.hex"
)
add_custom_command(TARGET ${MAIN_EXECUTABLE} POST_BUILD
    COMMAND ${CMAKE_OBJCOPY} -O binary $<TARGET_FILE:${MAIN_EXECUTABLE}> ${MAIN_EXECUTABLE}.bin
    BYPRODUCTS ${MAIN_EXECUTABLE}.bin
    COMMENT "Generating binary file: ${MAIN_EXECUTABLE}.bin"
)

set(MCUBOOT_IMGTOOL "${CMAKE_SOURCE_DIR}/modules/mcuboot/scripts/imgtool.py")
set(MCUBOOT_KEY "${CMAKE_SOURCE_DIR}/modules/mcuboot/root-ec-p256.pem")
set(FW_BIN "${CMAKE_BINARY_DIR}/${PROJECT_NAME}.bin")
set(SIGNED_FW_BIN "${CMAKE_BINARY_DIR}/${PROJECT_NAME}_signed.bin")

add_custom_command(
    OUTPUT ${SIGNED_FW_BIN}
    COMMAND python3 ${MCUBOOT_IMGTOOL} sign 
        --key ${MCUBOOT_KEY} 
        --header-size 0x200 
        --align 4 
        --slot-size 0x40000 
        --version ${FW_VERSION}
        --pad-header 
        ${FW_BIN} 
        ${SIGNED_FW_BIN}
    DEPENDS ${FW_BIN} ${MCUBOOT_IMGTOOL} ${MCUBOOT_KEY}
    COMMENT "MCUboot: Sign firmware ${PROJECT_NAME}.bin, version ${FW_VERSION}"
    VERBATIM
)

add_custom_target(sign_fw ALL DEPENDS ${SIGNED_FW_BIN})
