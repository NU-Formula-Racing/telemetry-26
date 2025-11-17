find_program(CLANG_FORMAT clang-format)
if (CLANG_FORMAT)
    file(GLOB_RECURSE ALL_CPP_FILES 
        ${CMAKE_SOURCE_DIR}/base_station/*.cpp
        ${CMAKE_SOURCE_DIR}/base_station/*.hpp
        ${CMAKE_SOURCE_DIR}/core/*.cpp
        ${CMAKE_SOURCE_DIR}/core/*.hpp
        ${CMAKE_SOURCE_DIR}/remote/*.cpp
        ${CMAKE_SOURCE_DIR}/remote/*.hpp
    )

    add_custom_target(format
        COMMENT "Running clang-format..."
        VERBATIM
    )

    # use root clang-format file
    set(CLANG_FORMAT_CONFIG_FILE "${CMAKE_SOURCE_DIR}/.clang-format")

    foreach(FILE ${ALL_CPP_FILES})
        add_custom_command(
            TARGET format
            POST_BUILD
            COMMAND ${CLANG_FORMAT} --dry-run -Werror ${FILE} --style=file:${CLANG_FORMAT_CONFIG_FILE}
            COMMENT "Formatting ${FILE}"
        )
    endforeach()
endif()