# =============================================================================
# OpenCV - Linux Platform
# =============================================================================

set(CV_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/x64")

# Find library
find_library(CV_LIB
    NAMES opencv_world libopencv_world.so
    PATHS "${CV_ROOT}/lib"
    NO_DEFAULT_PATH
)

# Fallback to system library if bundled not found
if(NOT CV_LIB)
    find_package(OpenCV QUIET)
    if(OpenCV_FOUND)
        message(STATUS "[OpenCV] Using system library")
        add_library(opencv INTERFACE)
        target_include_directories(opencv INTERFACE ${OpenCV_INCLUDE_DIRS})
        target_link_libraries(opencv INTERFACE ${OpenCV_LIBS})
        return()
    else()
        message(FATAL_ERROR "[OpenCV] Library not found. Install with: sudo apt install libopencv-dev")
    endif()
endif()

# Find shared libraries for copying
file(GLOB CV_SHARED_LIBS "${CV_ROOT}/lib/*.so*")

# Find include directory
set(CV_INCLUDE "${CV_ROOT}/include")
if(NOT EXISTS "${CV_INCLUDE}/opencv2/opencv.hpp")
    message(FATAL_ERROR "[OpenCV] Headers not found in ${CV_INCLUDE}")
endif()

message(STATUS "[OpenCV] Linux x64")
message(STATUS "[OpenCV] Library: ${CV_LIB}")
message(STATUS "[OpenCV] Shared libs: ${CV_SHARED_LIBS}")
message(STATUS "[OpenCV] Include: ${CV_INCLUDE}")

# Create copy target for shared libraries
if(CV_SHARED_LIBS)
    add_custom_target(copy_opencv)
    set_target_properties(copy_opencv PROPERTIES FOLDER framework/thirdparty/opencv)

    add_custom_command(TARGET copy_opencv POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/$<CONFIG>"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CV_SHARED_LIBS} "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/$<CONFIG>/"
        COMMAND_EXPAND_LISTS
        COMMENT "[OpenCV] Copying shared libraries to output directory"
    )
endif()

# Create interface library
add_library(opencv INTERFACE)
target_include_directories(opencv INTERFACE "${CV_INCLUDE}")
target_link_libraries(opencv INTERFACE "${CV_LIB}")

if(TARGET copy_opencv)
    add_dependencies(opencv copy_opencv)
endif()
