add_library(core_interfaces
            interface.hpp
            interface.cpp)

target_include_directories(core_interfaces INTERFACE 
                           ${CMAKE_CURRENT_SOURCE_DIR}
                          )