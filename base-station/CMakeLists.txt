add_subdirectory(src/app)

if (NOT CMAKE_CROSSCOMPILING AND CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_subdirectory(tests)
else()
    # add_executable(base_station.elf PRIVATE
    #               )               
    # create hex/bin file
endif()