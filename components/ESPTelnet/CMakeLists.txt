# ESPTelnet (ESPTelnet/ submodule)
set(root "${CMAKE_CURRENT_LIST_DIR}/ESPTelnet/src")

file(GLOB srcs "${root}/*.cpp")

idf_component_register(
    SRCS ${srcs}
    INCLUDE_DIRS "${root}"
    REQUIRES arduino
)
