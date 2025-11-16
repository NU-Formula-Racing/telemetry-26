add_library(remote_app  
            #list all files in app/
            app.hpp
            app.cpp
)

target_link_libraries(remote_app PRIVATE 
                       core_interfaces
                      )
