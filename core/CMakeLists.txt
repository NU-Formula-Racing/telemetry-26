add_subdirectory(interfaces)

if(CMAKE_CROSSCOMPILING)
    add_subdirectory(drivers)
endif()