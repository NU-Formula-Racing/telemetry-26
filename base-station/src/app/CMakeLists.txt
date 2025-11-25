add_library(base-station_app  
            #list all files in app/
            app.hpp
            app.cpp
)

target_link_libraries(base-station_app PRIVATE 
                       core_interfaces
                      )
