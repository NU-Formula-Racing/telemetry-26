add_subdirectory(interfaces)

if(BUILD_TARGET_STM)
    add_subdirectory(bsp)
    add_subdirectory(drivers)
endif()