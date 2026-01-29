# =============================================================================
# OpenCV - Android Platform
# =============================================================================

# Determine ABI directory
if(NOT ANDROID_ABI)
    message(FATAL_ERROR "[OpenCV] ANDROID_ABI not defined")
endif()

set(CV_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/${ANDROID_ABI}")
set(CV_LIB_DIR "${CV_ROOT}/libs")

if(NOT EXISTS "${CV_LIB_DIR}")
    message(FATAL_ERROR "[OpenCV] Android ABI directory not found: ${CV_LIB_DIR}")
endif()

# Find library (shared library)
find_library(CV_LIB
    NAMES opencv_world libopencv_world.so
    PATHS "${CV_LIB_DIR}"
    NO_DEFAULT_PATH
)

if(NOT CV_LIB)
    message(FATAL_ERROR "[OpenCV] Library not found in ${CV_LIB_DIR}")
endif()

# Find include directory (per-ABI)
set(CV_INCLUDE "${CV_ROOT}/include")
if(NOT EXISTS "${CV_INCLUDE}/opencv2/opencv.hpp")
    message(FATAL_ERROR "[OpenCV] Headers not found in ${CV_INCLUDE}")
endif()

message(STATUS "[OpenCV] Android ${ANDROID_ABI}")
message(STATUS "[OpenCV] Library: ${CV_LIB}")
message(STATUS "[OpenCV] Include: ${CV_INCLUDE}")

# Create interface library
add_library(opencv INTERFACE)
target_include_directories(opencv INTERFACE "${CV_INCLUDE}")
target_link_libraries(opencv INTERFACE "${CV_LIB}")
