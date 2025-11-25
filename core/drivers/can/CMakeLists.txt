add_library(core_can
            # list all can files
            can.hpp
            can.cpp
            )

target_include_directories(core_can PUBLIC 
                           ${CMAKE_CURRENT_SOURCE_DIR}
                          )