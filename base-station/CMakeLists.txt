cmake_minimum_required(VERSION 3.28)

project(telemetry-base-station 
        VERSION 0.0 
        LANGUAGES C CXX ASM)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

# only run linter on native builds
if(NOT CMAKE_CROSSCOMPILING)
    set(CMAKE_CXX_CLANG_TIDY "clang-tidy")
endif()

add_subdirectory(../core ${CMAKE_BINARY_DIR}/core)
add_subdirectory(src/app)

if(CMAKE_CROSSCOMPILING)
    message(STATUS "Building base-station for TARGET (STM32)...")
    if(NOT CMAKE_TOOLCHAIN_FILE)
        message(FATAL_ERROR "Target builds require a toolchain file. \
                             Set -DCMAKE_TOOLCHAIN_FILE=path/to/arm-none-eabi.cmake")
    endif()
    
    # compiler flags for STM32 - ARM Cortex-M4
    add_compile_options(-mcpu=cortex-m4
                        -mthumb
                        -mfpu=fpv4-sp-d16
                        -mfloat-abi=hard
                        -fdata-sections
                        -ffunction-sections
                        -Wall
                        -Wextra
                        $<$<CONFIG:Debug>:-Og>
                       )

    # linker flags for STM32 - ARM Cortex-M4
    add_link_options(-mcpu=cortex-m4
                     -mthumb
                     -mfpu=fpv4-sp-d16
                     -mfloat-abi=hard
                     --specs=nosys.specs 
                     # --specs=nano.specs # smaller binary size
                     -Wl,--gc-sections
                     -Wl,--print-memory-usage
                    # -T${CMAKE_CURRENT_SOURCE_DIR}/STM32F405RGTx_FLASH.ld
                    )

    # .elf target for base-station
    add_executable(base-station.elf
                   src/main.cpp)               

    target_include_directories(base-station.elf PRIVATE 
                               ${CMAKE_SOURCE_DIR}/src/app
                              )
 
    target_link_libraries(base-station.elf PRIVATE
                            base-station_app
                            core_bsp
                            core_drivers
                            core_interfaces
                         )
 
    # convert .elf to flashable .bin and .hex files with objcopy
    find_program(CMAKE_OBJCOPY NAMES arm-none-eabi-objcopy)
 
    add_custom_command(
        TARGET base-station.elf POST_BUILD
        COMMAND ${CMAKE_OBJCOPY} -O binary $<TARGET_FILE:base-station.elf> $<TARGET_FILE_DIR:base-station.elf>/base-station.bin
        COMMAND ${CMAKE_OBJCOPY} -O ihex $<TARGET_FILE:base-station.elf> $<TARGET_FILE_DIR:base-station.elf>/base-station.hex
        COMMENT "Generating base-station .bin and .hex files"
    )

else()
    message(STATUS "Building base-station for NATIVE (Host)...")
    
    # add unit testing + linting for native builds
    enable_testing()
    
    # disable linting for third-party code
    set(TEMP_CLANG_TIDY ${CMAKE_CXX_CLANG_TIDY})
    set(CMAKE_CXX_CLANG_TIDY "")
    add_subdirectory(../third-party/googletest ${CMAKE_BINARY_DIR}/googletest)
    set(CMAKE_CXX_CLANG_TIDY ${TEMP_CLANG_TIDY})

    include(GoogleTest)
    include(CTest)
    include(../tools/cmake/format.cmake)

    add_subdirectory(tests)
endif()
