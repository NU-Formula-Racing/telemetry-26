add_library(core_can INTERFACE
            # list all can files
            can.hpp
            # can_stm32.hpp
            )

target_include_directories(core_can INTERFACE
                           ${CMAKE_CURRENT_SOURCE_DIR}
                          )

if (CMAKE_CROSSCOMPILING)
    target_sources(core_can INTERFACE
                     can_stm32.hpp
                     )
    target_link_libraries(core_can INTERFACE 
                         platform_bsp
                         )
endif()

set_target_properties(core_can PROPERTIES LINKER_LANGUAGE CXX)