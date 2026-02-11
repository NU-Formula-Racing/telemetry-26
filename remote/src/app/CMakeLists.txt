add_library(remote_app  
            #list all files in app/
            # app.hpp
            # app.cpp
            logger.hpp
)

target_link_libraries(remote_app PRIVATE 
                      core_drivers
                      core_sd
                      core_rtc
)

# target_include_directories(remote_bsp PUBLIC 
#     ${CMAKE_CURRENT_SOURCE_DIR}/bsp
# )