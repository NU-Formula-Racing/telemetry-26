add_library(remote_app  
            # status.hpp
            logger.hpp
            wireless.hpp
            remote.hpp
)

target_link_libraries(remote_app PRIVATE 
                      core_drivers
                      core_sd
                      core_rtc
                      core_can
)

set_target_properties(remote_app PROPERTIES LINKER_LANGUAGE CXX)
