add_library(base_station_tests
            test.cpp
           )

target_include_directories(base_station_tests PRIVATE 
                           ${CMAKE_SOURCE_DIR}/base-station/src/app
                           ${CMAKE_SOURCE_DIR}/core/interfaces
                           ${CMAKE_SOURCE_DIR}/third-party/googletest/googletest
                           ${CMAKE_SOURCE_DIR}/third-party/googletest/googlemock
                          )

