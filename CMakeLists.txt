cmake_minimum_required(VERSION 3.22)

project(gas-lighter-fw LANGUAGES C CXX ASM)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON) 

set(MAIN_EXECUTABLE "gas-lighter-fw")
add_executable(${MAIN_EXECUTABLE})

file(GLOB_RECURSE APP_SOURCES 
    "${CMAKE_CURRENT_SOURCE_DIR}/app/src/*.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/app/src/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/freertos/*.c"
)

target_sources(${MAIN_EXECUTABLE} PRIVATE ${APP_SOURCES})

target_include_directories(${MAIN_EXECUTABLE} PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/app/inc"
)

set(TARGET_CMAKE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/targets/${TARGET}/cmake/stm32cubemx")

add_library(freertos_config INTERFACE)
target_include_directories(freertos_config SYSTEM INTERFACE
    ${CMAKE_SOURCE_DIR}/freertos
)

set( FREERTOS_HEAP "4" CACHE STRING "" FORCE)
set( FREERTOS_PORT GCC_ARM_CM4F)

add_subdirectory(${CMAKE_SOURCE_DIR}/modules/freertos)
add_subdirectory(RTT)
add_subdirectory(app)
add_subdirectory("${TARGET_CMAKE_DIR}" "${CMAKE_BINARY_DIR}/stm32cubemx_${TARGET}")

target_link_libraries(${MAIN_EXECUTABLE} freertos_kernel)
