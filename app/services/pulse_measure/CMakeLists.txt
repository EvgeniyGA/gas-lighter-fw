add_library(pulse_measure_service STATIC 
    src/pulse_measure.c
)

target_include_directories(pulse_measure_service PUBLIC 
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(pulse_measure_service PUBLIC 
    freertos_kernel
    bsp_drivers
)