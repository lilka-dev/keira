# Lua 5.4 (arduino-lua/ submodule)
set(root "${CMAKE_CURRENT_LIST_DIR}/arduino-lua")

# Same set as the library's srcFilter: the core and standard libraries, without the lua/luac executables
set(names
    lapi lcode lctype ldebug ldo ldump lfunc lgc llex lmem lobject lopcodes lparser lstate lstring
    ltable ltm lundump lvm lzio ltests lauxlib lbaselib ldblib liolib lmathlib loslib ltablib lstrlib
    lutf8lib loadlib lcorolib linit
)
list(TRANSFORM names PREPEND "${root}/src/" OUTPUT_VARIABLE srcs)
list(TRANSFORM srcs APPEND ".c")

idf_component_register(
    SRCS ${srcs}
    INCLUDE_DIRS "${root}/include" "${root}/src"
)
