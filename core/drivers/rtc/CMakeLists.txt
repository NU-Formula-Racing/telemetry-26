add_library(core_rtc INTERFACE
            rtc.hpp
            rtc_utils.hpp
            )

target_link_libraries(core_rtc INTERFACE
                       etl
)

target_include_directories(core_rtc INTERFACE
                           ${CMAKE_CURRENT_SOURCE_DIR}
                          )

# add stm stuff if compiling for target
if (CMAKE_CROSSCOMPILING)
    target_sources(core_rtc INTERFACE
                   rtc_stm32.hpp
                   )
    target_link_libraries(core_rtc INTERFACE platform_bsp)
endif()

set_target_properties(core_rtc PROPERTIES LINKER_LANGUAGE CXX)