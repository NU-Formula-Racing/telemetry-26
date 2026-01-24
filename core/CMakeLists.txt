add_subdirectory(interfaces)
add_subdirectory(tasks)
add_subdirectory(resources)
add_subdirectory(utils)

if(CMAKE_CROSSCOMPILING)
    add_subdirectory(drivers)
endif()