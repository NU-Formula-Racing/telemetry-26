add_subdirectory(interfaces)
add_subdirectory(tasks)
add_subdirectory(resources)

if(CMAKE_CROSSCOMPILING)
    add_subdirectory(drivers)
endif()