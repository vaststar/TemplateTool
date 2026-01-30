# =============================================================================
# libcurl - Linux Platform
# =============================================================================

set(CURL_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/x64")

# Find dynamic library (prefer dynamic)
find_library(CURL_LIB
    NAMES curl libcurl.so
    PATHS "${CURL_ROOT}/lib"
    NO_DEFAULT_PATH
)

# Fallback to system library if bundled not found
if(NOT CURL_LIB)
    find_package(CURL QUIET)
    if(CURL_FOUND)
        message(STATUS "[libcurl] Using system library")
        add_library(curl INTERFACE)
        target_link_libraries(curl INTERFACE CURL::libcurl)
        return()
    else()
        message(FATAL_ERROR "[libcurl] Library not found. Install with: sudo apt install libcurl4-openssl-dev")
    endif()
endif()

# Find shared libraries
file(GLOB CURL_SHARED_LIBS "${CURL_ROOT}/lib/libcurl.so*")

# Find include directory
set(CURL_INCLUDE "${CURL_ROOT}/include")
if(NOT EXISTS "${CURL_INCLUDE}/curl/curl.h")
    message(FATAL_ERROR "[libcurl] Headers not found in ${CURL_INCLUDE}")
endif()

message(STATUS "[libcurl] Linux x64")
message(STATUS "[libcurl] Library: ${CURL_LIB}")
message(STATUS "[libcurl] Shared libs: ${CURL_SHARED_LIBS}")
message(STATUS "[libcurl] Include: ${CURL_INCLUDE}")

# Create copy target for shared libraries
if(CURL_SHARED_LIBS)
    add_custom_target(copy_curl)
    set_target_properties(copy_curl PROPERTIES FOLDER framework/thirdparty/curl)
    
    add_custom_command(TARGET copy_curl POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CURL_SHARED_LIBS} "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/"
        COMMAND_EXPAND_LISTS
        COMMENT "[libcurl] Copying shared libraries to output directory"
    )
endif()

# Create interface library
add_library(curl INTERFACE)
target_include_directories(curl INTERFACE "${CURL_INCLUDE}")
target_link_libraries(curl INTERFACE "${CURL_LIB}")

if(TARGET copy_curl)
    add_dependencies(curl copy_curl)
endif()

# Install shared libraries to installation directory
if(CURL_SHARED_LIBS)
    install(FILES ${CURL_SHARED_LIBS} DESTINATION bin)
endif()
