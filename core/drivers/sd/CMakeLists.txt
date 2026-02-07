add_library(core_sd
            # list all sd files
            sd.hpp
            sd_stm32.hpp
            )

target_link_libraries(core_sd 
                      PUBLIC etl
                      PRIVATE platform_bsp)

target_include_directories(core_sd PUBLIC 
                           ${CMAKE_CURRENT_SOURCE_DIR}
                          )