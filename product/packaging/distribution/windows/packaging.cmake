# Configure Windows packaging; CPack generates configuration from this template
# using the shared version metadata pipeline in the distribution entry point.
set(TT_PLATFORM_CPACK_TEMPLATE
    "${CMAKE_CURRENT_LIST_DIR}/cpack_windows_config.cmake.in")
set(CPACK_NSIS_POLICY_FILE "${CMAKE_CURRENT_LIST_DIR}/nsis/policy.nsh")
include("${CMAKE_CURRENT_LIST_DIR}/nsis/template.cmake")
