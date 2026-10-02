# AnimatedGIF (AnimatedGIF/ submodule)
set(root "${CMAKE_CURRENT_LIST_DIR}/AnimatedGIF/src")

idf_component_register(
    SRCS "${root}/AnimatedGIF.cpp"
    INCLUDE_DIRS "${root}"
    REQUIRES arduino
)
