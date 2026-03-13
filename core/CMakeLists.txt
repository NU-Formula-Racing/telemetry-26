add_subdirectory(tasks)
add_subdirectory(utils)

if(CMAKE_CROSSCOMPILING)
    add_subdirectory(drivers)
endif()