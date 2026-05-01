add_library(core_can INTERFACE
            # list all can files
            can.hpp
            can_types.hpp
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
else()
    add_executable(can_test
                   can_test.cpp
                  )
    target_link_libraries(can_test PRIVATE
                          core_can
                          gtest 
                          gtest_main
                          )
    include(GoogleTest)
    gtest_discover_tests(can_test)
endif()

set_target_properties(core_can PROPERTIES LINKER_LANGUAGE CXX)