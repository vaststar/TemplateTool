# =============================================================================
# OpenCV - iOS Platform
# =============================================================================

set(CV_ROOT "${CMAKE_CURRENT_SOURCE_DIR}")

# iOS uses XCFramework
set(CV_XCFRAMEWORK "${CV_ROOT}/opencv2.xcframework")

if(NOT EXISTS "${CV_XCFRAMEWORK}")
    message(FATAL_ERROR "[OpenCV] opencv2.xcframework not found: ${CV_XCFRAMEWORK}")
endif()

# Find include directory
set(CV_INCLUDE "${CV_ROOT}/include")
if(NOT EXISTS "${CV_INCLUDE}/opencv2/opencv.hpp")
    message(FATAL_ERROR "[OpenCV] Headers not found in ${CV_INCLUDE}")
endif()

message(STATUS "[OpenCV] iOS")
message(STATUS "[OpenCV] XCFramework: ${CV_XCFRAMEWORK}")
message(STATUS "[OpenCV] Include: ${CV_INCLUDE}")

# Create interface library for iOS
add_library(opencv INTERFACE)
target_include_directories(opencv INTERFACE "${CV_INCLUDE}")
target_link_libraries(opencv INTERFACE "${CV_XCFRAMEWORK}")
