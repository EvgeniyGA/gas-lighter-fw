cmake_minimum_required(VERSION 3.22)

target_sources(${PROJECT_NAME} PRIVATE
    src/app.c
    src/msc_disk.c
    src/usb_descriptors.c
    src/retarget.c
)