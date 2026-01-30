# =============================================================================
# libcurl - macOS Platform
# =============================================================================

include(DetectPlatform)
detect_platform(PLATFORM_TYPE)

# Determine architecture directory
if(PLATFORM_TYPE STREQUAL "MAC_ARM")
    set(CURL_ARCH_DIR "arm")
elseif(PLATFORM_TYPE STREQUAL "MAC_INTEL")
    set(CURL_ARCH_DIR "intel")
else()
    message(FATAL_ERROR "[libcurl] Unsupported macOS architecture: ${PLATFORM_TYPE}")
endif()

set(CURL_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/${CURL_ARCH_DIR}")

# Find dynamic library (prefer dynamic for linking)
find_library(CURL_LIB
    NAMES curl libcurl.dylib
    PATHS "${CURL_ROOT}/lib"
    NO_DEFAULT_PATH
)

if(NOT CURL_LIB)
    message(FATAL_ERROR "[libcurl] Library not found in ${CURL_ROOT}/lib")
endif()

# Find dynamic library for runtime
file(GLOB CURL_DYLIBS "${CURL_ROOT}/lib/libcurl*.dylib")

# Find include directory
set(CURL_INCLUDE "${CURL_ROOT}/include")
if(NOT EXISTS "${CURL_INCLUDE}/curl/curl.h")
    message(FATAL_ERROR "[libcurl] Headers not found in ${CURL_INCLUDE}")
endif()

message(STATUS "[libcurl] macOS ${CURL_ARCH_DIR}")
message(STATUS "[libcurl] Library: ${CURL_LIB}")
message(STATUS "[libcurl] Dylibs: ${CURL_DYLIBS}")
message(STATUS "[libcurl] Include: ${CURL_INCLUDE}")

# Create copy target for dylibs
if(CURL_DYLIBS)
    add_custom_target(copy_curl)
    set_target_properties(copy_curl PROPERTIES FOLDER framework/thirdparty/curl)
    
    add_custom_command(TARGET copy_curl POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CURL_DYLIBS} "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/"
        COMMAND_EXPAND_LISTS
        COMMENT "[libcurl] Copying dylibs to output directory"
    )
endif()

# Create interface library
add_library(curl INTERFACE)
target_include_directories(curl INTERFACE "${CURL_INCLUDE}")
target_link_libraries(curl INTERFACE "${CURL_LIB}")

if(TARGET copy_curl)
    add_dependencies(curl copy_curl)
endif()

# Install dylibs to installation directory
if(CURL_DYLIBS)
    install(FILES ${CURL_DYLIBS} DESTINATION bin)
endif()
