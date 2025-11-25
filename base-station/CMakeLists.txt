cmake_minimum_required(VERSION 3.28)

project(telemetry-base-station 
        VERSION 0.0 
        LANGUAGES C CXX ASM)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

if(CMAKE_CROSSCOMPILING)
    message(STATUS "Configuring for STM32 TARGET...")

    if(NOT CMAKE_TOOLCHAIN_FILE)
        message(FATAL_ERROR "Target builds require a toolchain file.")
    endif()

    # set compile flags for STM32F4 (arm cortex-m4)
    add_compile_options(
        -mcpu=cortex-m4
        -mthumb
        -mfpu=fpv4-sp-d16
        -mfloat-abi=hard
        -fdata-sections
        -ffunction-sections
        -Wall
        -Wextra
        $<$<CONFIG:Debug>:-Og>
    )

    add_link_options(
        -mcpu=cortex-m4
        -mthumb
        -mfpu=fpv4-sp-d16
        -mfloat-abi=hard
        --specs=nosys.specs 
        -Wl,--gc-sections
        -Wl,--print-memory-usage
        # -T${CMAKE_CURRENT_SOURCE_DIR}/STM32F405RGTx_FLASH.ld
    )

else()
    message(STATUS "Configuring for NATIVE Host...")
    
    # only lint when building natively
    set(CMAKE_CXX_CLANG_TIDY "clang-tidy")
endif()

# add shared directories
add_subdirectory(../core ${CMAKE_BINARY_DIR}/core)
add_subdirectory(src/app)

if(CMAKE_CROSSCOMPILING)
    add_subdirectory(bsp)

    add_executable(base-station.elf src/main.cpp)               

    target_include_directories(base-station.elf PRIVATE 
                               ${CMAKE_CURRENT_SOURCE_DIR}/src/app
                               ${CMAKE_CURRENT_SOURCE_DIR}/bsp
                              )
 
    target_link_libraries(base-station.elf PRIVATE
                            base-station_app
                            base-station_bsp 
                            core_drivers
                            core_interfaces
                         )
 
    find_program(CMAKE_OBJCOPY NAMES arm-none-eabi-objcopy)
 
    add_custom_command(
        TARGET base-station.elf POST_BUILD
        COMMAND ${CMAKE_OBJCOPY} -O binary $<TARGET_FILE:base-station.elf> $<TARGET_FILE_DIR:base-station.elf>/base-station.bin
        COMMAND ${CMAKE_OBJCOPY} -O ihex $<TARGET_FILE:base-station.elf> $<TARGET_FILE_DIR:base-station.elf>/base-station.hex
        COMMENT "Generating base-station .bin and .hex files"
    )

else()
    enable_testing()
    
    # disable linter for GoogleTest
    set(TEMP_CLANG_TIDY ${CMAKE_CXX_CLANG_TIDY})
    set(CMAKE_CXX_CLANG_TIDY "")
    add_subdirectory(../third-party/googletest ${CMAKE_BINARY_DIR}/googletest)
    set(CMAKE_CXX_CLANG_TIDY ${TEMP_CLANG_TIDY})

    include(GoogleTest)
    include(CTest)
    include(../tools/cmake/format.cmake)

    add_subdirectory(tests)
endif()