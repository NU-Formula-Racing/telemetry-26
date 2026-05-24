add_library(base-station_app  
            #list all files in app/
            base_station.hpp
            wireless.hpp
)

target_link_libraries(base-station_app PRIVATE 
                       core_drivers
                      )

set_target_properties(base-station_app PROPERTIES LINKER_LANGUAGE CXX)
