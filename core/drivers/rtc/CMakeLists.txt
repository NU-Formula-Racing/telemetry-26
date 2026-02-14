add_library(core_rtc
            # list all rtc files
            rtc.hpp
            rtc_stm32.hpp
            rtc_utils.hpp
            )

target_link_libraries(core_rtc 
                      PUBLIC etl
                      PRIVATE platform_bsp)

target_include_directories(core_rtc PUBLIC 
                           ${CMAKE_CURRENT_SOURCE_DIR}
                          )