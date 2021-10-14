aux_source_directory(src DIR_SRCS)

add_library(ThreadPool ${DIR_SRCS})

target_include_directories(ThreadPool
    PUBLIC "inc")

set_target_properties(ThreadPool)