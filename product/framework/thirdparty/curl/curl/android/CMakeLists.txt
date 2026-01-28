# =============================================================================
# libcurl - Android Platform
# =============================================================================

# Determine ABI directory
if(NOT ANDROID_ABI)
    message(FATAL_ERROR "[libcurl] ANDROID_ABI not defined")
endif()

set(CURL_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/${ANDROID_ABI}")

if(NOT EXISTS "${CURL_ROOT}")
    message(FATAL_ERROR "[libcurl] Android ABI directory not found: ${CURL_ROOT}")
endif()

# Find dynamic library (prefer dynamic for Android)
find_library(CURL_LIB
    NAMES curl libcurl.so
    PATHS "${CURL_ROOT}/lib"
    NO_DEFAULT_PATH
)

if(NOT CURL_LIB)
    message(FATAL_ERROR "[libcurl] Library not found in ${CURL_ROOT}/lib")
endif()

# Find include directory
set(CURL_INCLUDE "${CURL_ROOT}/include")
if(NOT EXISTS "${CURL_INCLUDE}/curl/curl.h")
    message(FATAL_ERROR "[libcurl] Headers not found in ${CURL_INCLUDE}")
endif()

message(STATUS "[libcurl] Android ${ANDROID_ABI}")
message(STATUS "[libcurl] Library: ${CURL_LIB}")
message(STATUS "[libcurl] Include: ${CURL_INCLUDE}")

# Create interface library
add_library(curl INTERFACE)
target_include_directories(curl INTERFACE "${CURL_INCLUDE}")
target_link_libraries(curl INTERFACE "${CURL_LIB}")
