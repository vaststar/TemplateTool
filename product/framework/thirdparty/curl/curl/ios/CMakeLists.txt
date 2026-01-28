# =============================================================================
# libcurl - iOS Platform
# =============================================================================

set(CURL_ROOT "${CMAKE_CURRENT_SOURCE_DIR}")

# iOS uses XCFramework
set(CURL_XCFRAMEWORK "${CURL_ROOT}/libcurl.xcframework")

if(NOT EXISTS "${CURL_XCFRAMEWORK}")
    message(FATAL_ERROR "[libcurl] XCFramework not found: ${CURL_XCFRAMEWORK}")
endif()

# Find include directory
set(CURL_INCLUDE "${CURL_ROOT}/include")
if(NOT EXISTS "${CURL_INCLUDE}/curl/curl.h")
    message(FATAL_ERROR "[libcurl] Headers not found in ${CURL_INCLUDE}")
endif()

message(STATUS "[libcurl] iOS")
message(STATUS "[libcurl] XCFramework: ${CURL_XCFRAMEWORK}")
message(STATUS "[libcurl] Include: ${CURL_INCLUDE}")

# Create interface library for iOS
# XCFramework is handled by Xcode automatically
add_library(curl INTERFACE)
target_include_directories(curl INTERFACE "${CURL_INCLUDE}")

# Link the xcframework
# For CMake to properly link xcframework, we need to use the framework path
target_link_libraries(curl INTERFACE "${CURL_XCFRAMEWORK}")
