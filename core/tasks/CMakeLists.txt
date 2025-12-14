add_library(core_tasks INTERFACE)

target_sources(core_tasks INTERFACE
    job.hpp
    task.hpp
    # generator expression syntax: $<CONDITION:content_to_add>
    # $<$<BOOL:${CMAKE_CROSSCOMPILING}>:task.hpp>
)

target_include_directories(core_tasks INTERFACE 
                           ${CMAKE_CURRENT_SOURCE_DIR}
                          )

target_link_libraries(core_tasks INTERFACE etl)