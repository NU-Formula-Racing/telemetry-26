add_library(core_lora
            # list all lora files
            lora.hpp
            lora.cpp
            )

target_include_directories(core_lora PUBLIC 
                           ${CMAKE_CURRENT_SOURCE_DIR}
                          )