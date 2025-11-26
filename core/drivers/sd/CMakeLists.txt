add_library(core_sd
            # list all sd files
            sd.hpp
            sd.cpp
            )

target_link_libraries(core_sd PRIVATE platform_bsp)

target_include_directories(core_sd PUBLIC 
                           ${CMAKE_CURRENT_SOURCE_DIR}
                          )