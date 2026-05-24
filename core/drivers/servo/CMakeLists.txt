add_library(core_servo INTERFACE
            servo.hpp
            )

target_include_directories(core_servo INTERFACE
                           ${CMAKE_CURRENT_SOURCE_DIR}
                        )

if (CMAKE_CROSSCOMPILING)
    target_sources(core_servo INTERFACE
                     servo_stm32.hpp
                     )
    target_link_libraries(core_servo INTERFACE
                         platform_bsp
                         )
endif()

# set_target_properties(core_servo PROPERTIES LINKER_LANGUAGE CXX)