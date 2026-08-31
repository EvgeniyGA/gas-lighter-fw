if(NOT DEFINED MAIN_TIME_RESOLUTION)
    message(FATAL_ERROR 
    "MAIN_TIME_RESOLUTION is not defined. "
    "Please, set in in root CMakeLists.txt or by -DMAIN_TIME_RESOLUTION=...")
endif()

add_library(wave_starter_service STATIC 
    include/wave_starter.h
    src/wave_gen.h
    src/wave_gen.c 
    src/wave_starter.c 
)

target_compile_definitions(wave_starter_service PRIVATE
    MAIN_TIME_RESOLUTION=${MAIN_TIME_RESOLUTION}
)

target_include_directories(wave_starter_service PUBLIC 
    include
)

target_include_directories(wave_starter_service PUBLIC 
    src
)

target_link_libraries(wave_starter_service PUBLIC 
    bsp_drivers
)