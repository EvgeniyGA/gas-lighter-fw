add_library(wave_measure_service STATIC
    src/wave_measure.c
)

target_include_directories(wave_measure_service PUBLIC
    include
)

target_link_libraries(wave_measure_service 
    freertos_kernel
    CMSISDSP
)