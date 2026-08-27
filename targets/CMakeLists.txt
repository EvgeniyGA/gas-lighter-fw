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
