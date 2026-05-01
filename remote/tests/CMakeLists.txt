add_executable(remote_tests
               odometer_test.cpp
               # wireless_test.cpp
               # logger_test.cpp
               # remote_test.cpp
              )

target_include_directories(remote_tests PRIVATE 
                           ${CMAKE_SOURCE_DIR}/src/app
                           ${CMAKE_SOURCE_DIR}/../core/interfaces
                           ${CMAKE_SOURCE_DIR}/third-party/googletest/googletest
                           ${CMAKE_SOURCE_DIR}/third-party/googletest/googlemock
                          )

target_link_libraries(remote_tests 
                      remote_app
                      gtest 
                      gtest_main
                      )

gtest_discover_tests(remote_tests)