cmake_minimum_required(VERSION 3.28)

project(telemetry-remote 
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
        -Wno-psabi
        --specs=nano.specs 
        $<$<CONFIG:Debug>:-Og>
    )

    add_link_options(
        -mcpu=cortex-m4
        -mthumb
        -mfpu=fpv4-sp-d16
        -mfloat-abi=hard
        --specs=nano.specs 
        -Wl,--gc-sections
        -Wl,--print-memory-usage
    )

    set( FREERTOS_HEAP "4" CACHE STRING "" FORCE)
    # Select the native compile PORT
    set( FREERTOS_PORT "GCC_POSIX" CACHE STRING "" FORCE)
    # Select the cross-compile PORT
    set(FREERTOS_PORT "GCC_ARM_CM4F" CACHE STRING "" FORCE)

    add_subdirectory(bsp)
    add_library(platform_bsp ALIAS remote_bsp) # inject remote bsp alias so core can find it
    target_compile_options(remote_bsp PRIVATE -w) # turn off warnings for STM32CubeMX generated code
else()
    message(STATUS "Configuring for NATIVE Host...")
    
    # only lint when building natively
    set(CMAKE_CXX_CLANG_TIDY "clang-tidy")
endif()

# add shared directories
add_subdirectory(../core ${CMAKE_BINARY_DIR}/core)
add_subdirectory(src/app)
add_subdirectory(../third-party/etl ${CMAKE_BINARY_DIR}/etl)
add_subdirectory(../third-party/nfrcanlib ${CMAKE_BINARY_DIR}/nfrcanlib)

if(CMAKE_CROSSCOMPILING)
    # build .elf for target
    add_executable(remote.elf src/main.cpp
    bsp/startup_stm32f405xx.s
    )               

    target_include_directories(remote.elf PRIVATE 
                               ${CMAKE_CURRENT_SOURCE_DIR}/src/app
                               ${CMAKE_CURRENT_SOURCE_DIR}/bsp/Core/Inc
                               ${CMAKE_CURRENT_SOURCE_DIR}/bsp/FATFS
                               ${CMAKE_CURRENT_SOURCE_DIR}/bsp
                               ${CMAKE_CURRENT_SOURCE_DIR}/../core
                               ${CMAKE_CURRENT_SOURCE_DIR}/../third-party/nfrcanlib/include
                              )
 
    target_link_libraries(remote.elf PRIVATE
                            remote_app
                            remote_bsp 
                            core_drivers
                            core_tasks
                            freertos_kernel
                            freertos_config
                            nfr_canlib
                         )

    add_subdirectory(../third-party/freertos ${CMAKE_BINARY_DIR}/freertos)

    # generate .bin and .hex from .elf using objcopy
    find_program(CMAKE_OBJCOPY NAMES arm-none-eabi-objcopy)
 
    add_custom_command(
        TARGET remote.elf POST_BUILD
        COMMAND ${CMAKE_OBJCOPY} -O binary $<TARGET_FILE:remote.elf> $<TARGET_FILE_DIR:remote.elf>/remote.bin
        COMMAND ${CMAKE_OBJCOPY} -O ihex $<TARGET_FILE:remote.elf> $<TARGET_FILE_DIR:remote.elf>/remote.hex
        COMMENT "Generated remote .bin and .hex files"
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

    add_subdirectory(tests)
endif()