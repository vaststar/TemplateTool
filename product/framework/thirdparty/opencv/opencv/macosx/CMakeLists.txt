# =============================================================================
# OpenCV - macOS Platform
# =============================================================================

include(DetectPlatform)
detect_platform(PLATFORM_TYPE)

# Determine architecture directory
if(PLATFORM_TYPE STREQUAL "MAC_ARM")
    set(CV_ARCH_DIR "arm")
elseif(PLATFORM_TYPE STREQUAL "MAC_INTEL")
    set(CV_ARCH_DIR "intel")
else()
    message(FATAL_ERROR "[OpenCV] Unsupported macOS architecture: ${PLATFORM_TYPE}")
endif()

set(CV_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/${CV_ARCH_DIR}")

# Find library
find_library(CV_LIB
    NAMES opencv_world libopencv_world.dylib
    PATHS "${CV_ROOT}/lib"
    NO_DEFAULT_PATH
)

if(NOT CV_LIB)
    message(FATAL_ERROR "[OpenCV] Library not found in ${CV_ROOT}/lib")
endif()

# Find dylibs for copying
file(GLOB CV_DYLIBS "${CV_ROOT}/lib/*.dylib")

# Find include directory (per-architecture)
set(CV_INCLUDE "${CV_ROOT}/include")
if(NOT EXISTS "${CV_INCLUDE}/opencv2/opencv.hpp")
    message(FATAL_ERROR "[OpenCV] Headers not found in ${CV_INCLUDE}")
endif()

message(STATUS "[OpenCV] macOS ${CV_ARCH_DIR}")
message(STATUS "[OpenCV] Library: ${CV_LIB}")
message(STATUS "[OpenCV] Dylibs: ${CV_DYLIBS}")
message(STATUS "[OpenCV] Include: ${CV_INCLUDE}")

# Create copy target for dylibs
if(CV_DYLIBS)
    add_custom_target(copy_opencv)
    set_target_properties(copy_opencv PROPERTIES FOLDER framework/thirdparty/opencv)

    add_custom_command(TARGET copy_opencv POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/$<CONFIG>"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CV_DYLIBS} "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/$<CONFIG>/"
        COMMAND_EXPAND_LISTS
        COMMENT "[OpenCV] Copying dylibs to output directory"
    )
endif()

# Create interface library
add_library(opencv INTERFACE)
target_include_directories(opencv INTERFACE "${CV_INCLUDE}")
target_link_libraries(opencv INTERFACE "${CV_LIB}")

if(TARGET copy_opencv)
    add_dependencies(opencv copy_opencv)
endif()

# Install dylibs to installation directory
if(CV_DYLIBS)
    install(FILES ${CV_DYLIBS} DESTINATION bin)
endif()
