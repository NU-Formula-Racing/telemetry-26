add_library(base-station_app STATIC)

target_sources(base-station_app
    PRIVATE
        base_station.cpp
    PUBLIC
        wireless.hpp
        base_station.hpp
)

target_include_directories(base-station_app PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/../../../core
)

target_link_libraries(base-station_app PRIVATE 
                       core_drivers
                      )

set_target_properties(base-station_app PROPERTIES LINKER_LANGUAGE CXX)
