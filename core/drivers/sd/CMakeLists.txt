add_library(core_sd
            # list all sd files
            sd.hpp
            sd.cpp
            )

target_include_directories(core_sd PUBLIC 
                           ${CMAKE_CURRENT_SOURCE_DIR}
                          )