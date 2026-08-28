cmake_minimum_required(VERSION 3.22)

target_sources(${PROJECT_NAME} PRIVATE
    core/src/app.c
    core/src/retarget.c
    core/src/wrapper.c
    core/src/cli_commands.c
    core/src/cli_commands_fs.c
    core/src/usb_service.c
    core/src/process_adc.c
    core/src/pulse_measure.c
    core/src/wave_gen.c
#    core/src/wave_measure.c
    core/src/wave_starter.c
    core/src/version_check.c
)

add_subdirectory(services)