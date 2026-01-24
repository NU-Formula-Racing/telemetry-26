add_library(remote_app  
            #list all files in app/
            app.hpp
            app.cpp
)

target_link_libraries(remote_app PRIVATE 
                       core_interfaces
                      )

# target_include_directories(remote_bsp PUBLIC 
#     ${CMAKE_CURRENT_SOURCE_DIR}/bsp
# )