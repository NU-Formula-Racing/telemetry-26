add_library(core_usb INTERFACE
            # list all usb files
            usb.hpp
            usb_stm32.hpp
            )

target_include_directories(core_usb INTERFACE
                           ${CMAKE_CURRENT_SOURCE_DIR}
                        )

if (CMAKE_CROSSCOMPILING)
    target_sources(core_usb INTERFACE
                     usb_stm32.hpp
                     )
    target_link_libraries(core_usb INTERFACE
                         platform_bsp
                         )
# else()
#     add_executable(usb_test
#                    usb_test.cpp
#                   )
#     target_link_libraries(usb_test PRIVATE
#                           core_usb
#                           gtest 
#                           gtest_main
#                           )
#     include(GoogleTest)
#     gtest_discover_tests(usb_test)
endif()

set_target_properties(core_usb PROPERTIES LINKER_LANGUAGE CXX)