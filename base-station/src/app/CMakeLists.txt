add_library(base_station_app  
            #list all files in app/
            app.hpp
            app.cpp
)

target_link_libraries(base_station_app PRIVATE 
                       core_interfaces
                      )
