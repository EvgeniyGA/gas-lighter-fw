cmake_minimum_required(VERSION 3.19)

if(NOT TARGET fatfs_config)
    message(FATAL_ERROR "fatfs_config target not specified")
endif()

set(FATFS_SOURCE_DIR ${CMAKE_SOURCE_DIR}/modules/fatfs/source)

add_library(fatfs_core STATIC)

target_sources(fatfs_core PRIVATE
    ${FATFS_SOURCE_DIR}/ff.c
    ${FATFS_SOURCE_DIR}/ff_gen_drv.c
    ${FATFS_SOURCE_DIR}/diskio.c
    ${CMAKE_CURRENT_SOURCE_DIR}/fatfs.c
    ${CMAKE_CURRENT_SOURCE_DIR}/drivers/sram_diskio.c
    ${CMAKE_CURRENT_SOURCE_DIR}/drivers/flash_diskio.c
)

target_include_directories(fatfs_core PUBLIC
    ${FATFS_SOURCE_DIR}  
    ${FATFS_SOURCE_DIR}/drivers
    ${CMAKE_CURRENT_SOURCE_DIR} 
)

target_link_libraries(fatfs_core PUBLIC fatfs_config)