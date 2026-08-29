add_library(freertos_support STATIC 
    freertos.c
    freertos_hook.c
)

# if(${TARGET} STREQUAL "stm32f4")
#     set( FREERTOS_PORT GCC_ARM_CM4F CACHE STRING "FreeRTOS port" FORCE)
# elseif(${TARGET} STREQUAL "stm32f7")
#     set( FREERTOS_PORT GCC_ARM_CM7 CACHE STRING "FreeRTOS port" FORCE)
# else()
#     message(FATAL_ERROR "Неизвестный TARGET: '${TARGET}'. Добавьте настройку FREERTOS_PORT для этой платформы.")
# endif()

#set( FREERTOS_HEAP "4" CACHE STRING "" FORCE)

#target_link_libraries(freertos_support INTERFACE system_viewer)
target_link_libraries(freertos_support PUBLIC 
    freertos_config
)

target_include_directories(freertos_support PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)