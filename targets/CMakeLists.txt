set(BSP_SOURCES
    ${TARGET}/Core/Src/adc_driver.c
    ${TARGET}/Core/Src/dac_driver.c
     ${TARGET}/Core/Src/W25Qxx.c
)

add_library(bsp_drivers STATIC 
    ${BSP_SOURCES}
)

target_link_libraries(bsp_drivers PUBLIC 
    stm32cubemx
)

target_include_directories(bsp_drivers PUBLIC 
    ${TARGET}/Core/Inc
)

add_library(cmsis_core_headers INTERFACE)

if(${TARGET} STREQUAL "stm32f4")
    set(CMSIS_DEVICE_FAMILY "STM32F4xx")
elseif(${TARGET} STREQUAL "stm32f7")
    set(CMSIS_DEVICE_FAMILY "STM32F7xx")
else()
    message(FATAL_ERROR "Неизвестный TARGET: '${TARGET}'")
endif()

target_include_directories(cmsis_core_headers INTERFACE
    ${TARGET}/Drivers/CMSIS/Include
    ${TARGET}/Drivers/CMSIS/Device/ST/${CMSIS_DEVICE_FAMILY}/Include
)

target_include_directories(CMSISDSP PRIVATE
    ${TARGET}/Drivers/CMSIS/Include
    ${TARGET}/Drivers/CMSIS/Device/ST/${CMSIS_DEVICE_FAMILY}/Include
)

