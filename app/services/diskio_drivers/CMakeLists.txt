add_library(diskio_drivers STATIC 
    drivers/flash_diskio.c
    drivers/sram_diskio.c
)

target_include_directories(diskio_drivers PUBLIC 
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/drivers
)

target_link_libraries(diskio_drivers PUBLIC 
    fatfs_core
)