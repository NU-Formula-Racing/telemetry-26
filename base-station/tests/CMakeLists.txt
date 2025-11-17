add_executable(base_station_tests
            test.cpp
           )

target_include_directories(base_station_tests PRIVATE 
                           ${CMAKE_SOURCE_DIR}/base-station/src/app
                           ${CMAKE_SOURCE_DIR}/core/interfaces
                           ${CMAKE_SOURCE_DIR}/third-party/googletest/googletest
                           ${CMAKE_SOURCE_DIR}/third-party/googletest/googlemock
                          )

target_link_libraries(base_station_tests 
                      base_station_app
                      gtest 
                      gtest_main
                      )

gtest_discover_tests(base_station_tests)