add_executable(base-station_tests
               test.cpp
              )

target_include_directories(base-station_tests PRIVATE 
                           ${CMAKE_SOURCE_DIR}/src/app
                           ${CMAKE_SOURCE_DIR}/../core/interfaces
                           ${CMAKE_SOURCE_DIR}/third-party/googletest/googletest
                           ${CMAKE_SOURCE_DIR}/third-party/googletest/googlemock
                          )

target_link_libraries(base-station_tests 
                      base-station_app
                      core_tasks
                      gtest 
                      gtest_main
                      )

gtest_discover_tests(base-station_tests)