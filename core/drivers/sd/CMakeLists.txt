add_library(core_sd INTERFACE
            sd.hpp
            )

target_link_libraries(core_sd INTERFACE
                       etl
)

target_include_directories(core_sd INTERFACE
                           ${CMAKE_CURRENT_SOURCE_DIR}
                          )

if (CMAKE_CROSSCOMPILING)
    target_sources(core_sd INTERFACE
                   sd_stm32.hpp
                   )
    target_link_libraries(core_sd INTERFACE platform_bsp)
endif()

set_target_properties(core_sd PROPERTIES LINKER_LANGUAGE CXX)