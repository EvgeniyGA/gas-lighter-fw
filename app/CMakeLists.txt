cmake_minimum_required(VERSION 3.22)

target_include_directories(${PROJECT_NAME} PUBLIC 
    ${CMAKE_CURRENT_SOURCE_DIR}/core/inc
)

target_sources(${PROJECT_NAME} PRIVATE
    core/src/app.cpp
    core/src/retarget.c
    core/src/wrapper.c
    core/src/version_check.c
    core/src/no_heap.cpp
)

add_subdirectory(services)

if(${TARGET} STREQUAL "stm32f4")
    message(STATUS "configure gaselighter device")
    add_subdirectory(gase_tester)
elseif(${TARGET} STREQUAL "stm32f7")
    message(STATUS "configure ns_generator device")
    add_subdirectory(ns_gen)
else()
    message(FATAL_ERROR "unsupported target ${TARGET}")
endif()
