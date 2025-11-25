add_subdirectory(interfaces)

if(CMAKE_CROSSCOMPILING)
    add_subdirectory(bsp)
    add_subdirectory(drivers)
endif()