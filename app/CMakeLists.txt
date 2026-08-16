cmake_minimum_required(VERSION 3.22)

target_sources(${PROJECT_NAME} PRIVATE
    src/app.c
    src/retarget.c
    src/wrapper.c
    src/cli_commands.c
    src/cli_commands_fs.c
    src/usb_service.c
    src/version_check.c
    src/lcd_printer.c
)