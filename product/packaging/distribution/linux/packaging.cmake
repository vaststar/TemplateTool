# Declare the Linux integration hook before the common staging template embeds
# its generated path. Keep generated files in the distribution build directory.
set(TT_PLATFORM_CPACK_TEMPLATE
    "${CMAKE_CURRENT_LIST_DIR}/cpack_linux_config.cmake.in")
set(CPACK_LINUX_PRE_BUILD_SCRIPT
    "${CMAKE_CURRENT_BINARY_DIR}/cpack_linux_pre_build.cmake")
configure_file(
    "${CMAKE_CURRENT_LIST_DIR}/cpack_linux_pre_build.cmake.in"
    "${CPACK_LINUX_PRE_BUILD_SCRIPT}"
    @ONLY
)
