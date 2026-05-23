add_library(remote_app STATIC)

target_sources(remote_app
    PRIVATE
        wireless.cpp
    PUBLIC
        can_msgs.hpp
        odometer.hpp
        logger.hpp
        wireless.hpp
        remote.hpp
)

target_include_directories(remote_app PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/../../../core
)

target_link_libraries(remote_app PUBLIC
                      core_drivers
                      core_sd
                      core_rtc
                      core_can
)

if (CMAKE_CROSSCOMPILING)
    target_link_libraries(remote_app PRIVATE freertos_kernel freertos_config)
endif()

# set_target_properties(remote_app PROPERTIES LINKER_LANGUAGE CXX)
