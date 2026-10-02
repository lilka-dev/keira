# SimpleFTPServer (vendored in src/)
set(root "${CMAKE_CURRENT_LIST_DIR}/src")

idf_component_register(
    SRCS "${root}/FtpServer.cpp"
    INCLUDE_DIRS "${root}"
    REQUIRES arduino
)
