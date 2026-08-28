if(NOT DEFINED MAIN_TIME_RESOLUTION)
    message(FATAL_ERROR 
    "MAIN_TIME_RESOLUTION is not defined. "
    "Please, set in in root CMakeLists.txt or by -DMAIN_TIME_RESOLUTION=...")
endif()

add_library(wave_measure_service STATIC
    src/wave_measure.c
)

target_compile_definitions(wave_measure_service PRIVATE
    MAIN_TIME_RESOLUTION=${MAIN_TIME_RESOLUTION}
)

target_include_directories(wave_measure_service PUBLIC
    include
)

target_link_libraries(wave_measure_service 
    freertos_kernel
    CMSISDSP
    cmsis_core_headers
    m
)