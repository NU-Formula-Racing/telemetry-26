add_library(remote_tests
            test.cpp
           )

target_include_directories(remote_tests PRIVATE 
                           ${CMAKE_SOURCE_DIR}/remote/src/app
                           ${CMAKE_SOURCE_DIR}/core/interfaces
                           ${CMAKE_SOURCE_DIR}/third-party/googletest/googletest
                           ${CMAKE_SOURCE_DIR}/third-party/googletest/googlemock
                          )

