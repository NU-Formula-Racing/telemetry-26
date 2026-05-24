add_library(core_usb INTERFACE
            usb.hpp
            )

target_include_directories(core_usb INTERFACE
                            ${CMAKE_CURRENT_SOURCE_DIR}
                              )

if (CMAKE_CROSSCOMPILING)
    target_sources(core_usb INTERFACE
                   usb_stm32.hpp
                   )
    target_link_libraries(core_usb INTERFACE platform_bsp)
endif()

set_target_properties(core_usb PROPERTIES LINKER_LANGUAGE CXX)