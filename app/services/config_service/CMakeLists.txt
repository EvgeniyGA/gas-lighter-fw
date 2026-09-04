add_library(config_service STATIC 
    src/config_service.c
)

target_include_directories(config_service PUBLIC 
    include
)