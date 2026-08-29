cmake_minimum_required(VERSION 3.22)

target_include_directories(${PROJECT_NAME} PUBLIC 
    ${CMAKE_CURRENT_SOURCE_DIR}/core/inc
)

target_sources(${PROJECT_NAME} PRIVATE
    core/src/app.c
    core/src/retarget.c
    core/src/wrapper.c
    core/src/cli_commands.c
    core/src/cli_commands_fs.c
    core/src/usb_service.c
    core/src/pulse_measure.c
    core/src/wave_gen.c
    core/src/wave_starter.c
    core/src/version_check.c
)

add_subdirectory(services)