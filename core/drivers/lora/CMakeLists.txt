add_library(core_lora INTERFACE
            lora.hpp
            rfm95_utils.hpp
            packet_types.hpp
            )

target_link_libraries(core_lora INTERFACE
                        etl 
                      )

target_include_directories(core_lora INTERFACE
                           ${CMAKE_CURRENT_SOURCE_DIR}
                          )

if (CMAKE_CROSSCOMPILING)
    target_sources(core_lora INTERFACE
                   rfm95.hpp
                   spi.hpp
                   )
    target_link_libraries(core_lora INTERFACE platform_bsp)
endif()

set_target_properties(core_lora PROPERTIES LINKER_LANGUAGE CXX)