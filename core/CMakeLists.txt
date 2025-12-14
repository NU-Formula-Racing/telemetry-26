add_subdirectory(interfaces)
add_subdirectory(tasks)

if(CMAKE_CROSSCOMPILING)
    add_subdirectory(drivers)
endif()