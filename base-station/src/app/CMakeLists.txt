add_library(base-station_app  
            #list all files in app/
            base_station.hpp
)

target_link_libraries(base-station_app PRIVATE 
                       core_drivers
                      )
