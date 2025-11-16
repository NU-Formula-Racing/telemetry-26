add_subdirectory(interfaces)

if(BUILD_TARGET_HW)
    add_subdirectory(bsp)
    add_subdirectory(drivers)
endif()