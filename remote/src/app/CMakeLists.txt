add_library(remote_app INTERFACE)

target_sources(remote_app INTERFACE 
            can_signals.hpp
            can_msgs.hpp
            odometer.hpp
            logger.hpp
            wireless.hpp
            remote.hpp
)

target_link_libraries(remote_app INTERFACE
                      core_drivers
                      core_sd
                      core_rtc
                      core_can
)

# set_target_properties(remote_app PROPERTIES LINKER_LANGUAGE CXX)
