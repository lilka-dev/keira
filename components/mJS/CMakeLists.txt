# mJS (vendored in src/)
set(root "${CMAKE_CURRENT_LIST_DIR}/src")

idf_component_register(
    SRCS "${root}/mjs.c"
    INCLUDE_DIRS "${root}"
)
