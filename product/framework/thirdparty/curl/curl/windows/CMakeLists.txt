# =============================================================================
# libcurl - Windows Platform
# =============================================================================

include(DetectPlatform)
detect_platform(PLATFORM_TYPE)

# Determine architecture directory
if(PLATFORM_TYPE STREQUAL "WINDOWS_X64")
    set(CURL_ARCH_DIR "x64")
elseif(PLATFORM_TYPE STREQUAL "WINDOWS_X86")
    set(CURL_ARCH_DIR "x86")
elseif(PLATFORM_TYPE STREQUAL "WINDOWS_ARM64")
    set(CURL_ARCH_DIR "arm64")
else()
    message(FATAL_ERROR "[libcurl] Unsupported Windows architecture: ${PLATFORM_TYPE}")
endif()

set(CURL_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/${CURL_ARCH_DIR}")

# Find library
find_library(CURL_LIB
    NAMES libcurl_imp libcurl
    PATHS "${CURL_ROOT}/lib"
    NO_DEFAULT_PATH
)

if(NOT CURL_LIB)
    message(FATAL_ERROR "[libcurl] Library not found in ${CURL_ROOT}/lib")
endif()

# Find DLLs
file(GLOB CURL_DLLS "${CURL_ROOT}/bin/*.dll")

if(NOT CURL_DLLS)
    message(FATAL_ERROR "[libcurl] DLLs not found in ${CURL_ROOT}/bin")
endif()

# Find include directory
set(CURL_INCLUDE "${CURL_ROOT}/include")
if(NOT EXISTS "${CURL_INCLUDE}/curl/curl.h")
    message(FATAL_ERROR "[libcurl] Headers not found in ${CURL_INCLUDE}")
endif()

message(STATUS "[libcurl] Windows ${CURL_ARCH_DIR}")
message(STATUS "[libcurl] Library: ${CURL_LIB}")
message(STATUS "[libcurl] DLLs: ${CURL_DLLS}")
message(STATUS "[libcurl] Include: ${CURL_INCLUDE}")

# Create copy target
add_custom_target(copy_curl)
set_target_properties(copy_curl PROPERTIES FOLDER framework/thirdparty/curl)

add_custom_command(TARGET copy_curl POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/$<CONFIG>"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CURL_DLLS} "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/$<CONFIG>/"
    COMMAND_EXPAND_LISTS
    COMMENT "[libcurl] Copying DLLs to output directory"
)

# Create interface library
add_library(curl INTERFACE)
target_include_directories(curl INTERFACE "${CURL_INCLUDE}")
target_link_libraries(curl INTERFACE "${CURL_LIB}")
add_dependencies(curl copy_curl)

# Install DLLs to installation directory
install(FILES ${CURL_DLLS} DESTINATION bin)
