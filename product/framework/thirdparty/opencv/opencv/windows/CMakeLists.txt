# =============================================================================
# OpenCV - Windows Platform
# =============================================================================

include(DetectPlatform)
detect_platform(PLATFORM_TYPE)

# Determine architecture directory
if(PLATFORM_TYPE STREQUAL "WINDOWS_X64")
    set(CV_ARCH_DIR "x64")
elseif(PLATFORM_TYPE STREQUAL "WINDOWS_X86")
    set(CV_ARCH_DIR "x86")
else()
    message(FATAL_ERROR "[OpenCV] Unsupported Windows architecture: ${PLATFORM_TYPE}")
endif()

set(CV_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/${CV_ARCH_DIR}/vc17")

# Find libraries (Release and Debug)
find_library(CV_LIB_RELEASE
    NAMES opencv_world4130
    PATHS "${CV_ROOT}/lib"
    NO_DEFAULT_PATH
)

find_library(CV_LIB_DEBUG
    NAMES opencv_world4130d
    PATHS "${CV_ROOT}/lib"
    NO_DEFAULT_PATH
)

if(NOT CV_LIB_RELEASE)
    message(FATAL_ERROR "[OpenCV] Release library not found in ${CV_ROOT}/lib")
endif()

if(NOT CV_LIB_DEBUG)
    message(STATUS "[OpenCV] Debug library not found, using Release for Debug builds")
    set(CV_LIB_DEBUG "${CV_LIB_RELEASE}")
endif()

# Find DLLs
file(GLOB CV_DLLS_RELEASE "${CV_ROOT}/bin/opencv_world4130.dll" "${CV_ROOT}/bin/opencv_videoio_ffmpeg4130_64.dll")
file(GLOB CV_DLLS_DEBUG "${CV_ROOT}/bin/opencv_world4130d.dll" "${CV_ROOT}/bin/opencv_videoio_ffmpeg4130_64.dll")

if(NOT CV_DLLS_RELEASE)
    message(FATAL_ERROR "[OpenCV] DLLs not found in ${CV_ROOT}/bin")
endif()

# Find include directory (per-architecture)
set(CV_INCLUDE "${CMAKE_CURRENT_SOURCE_DIR}/${CV_ARCH_DIR}/include")
if(NOT EXISTS "${CV_INCLUDE}/opencv2/opencv.hpp")
    message(FATAL_ERROR "[OpenCV] Headers not found in ${CV_INCLUDE}")
endif()

message(STATUS "[OpenCV] Windows ${CV_ARCH_DIR}")
message(STATUS "[OpenCV] Release lib: ${CV_LIB_RELEASE}")
message(STATUS "[OpenCV] Debug lib: ${CV_LIB_DEBUG}")
message(STATUS "[OpenCV] Include: ${CV_INCLUDE}")

# Create copy target
add_custom_target(copy_opencv)
set_target_properties(copy_opencv PROPERTIES FOLDER framework/thirdparty/opencv)

add_custom_command(TARGET copy_opencv POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/$<CONFIG>"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different 
        "$<IF:$<CONFIG:Debug>,${CV_DLLS_DEBUG},${CV_DLLS_RELEASE}>"
        "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/$<CONFIG>/"
    COMMAND_EXPAND_LISTS
    COMMENT "[OpenCV] Copying DLLs to output directory"
)

# Create interface library
add_library(opencv INTERFACE)
target_include_directories(opencv INTERFACE "${CV_INCLUDE}")
target_link_libraries(opencv INTERFACE 
    "$<IF:$<CONFIG:Debug>,${CV_LIB_DEBUG},${CV_LIB_RELEASE}>"
)
add_dependencies(opencv copy_opencv)

# Install DLLs to installation directory
install(FILES ${CV_DLLS_RELEASE} DESTINATION bin CONFIGURATIONS Release RelWithDebInfo MinSizeRel)
install(FILES ${CV_DLLS_DEBUG} DESTINATION bin CONFIGURATIONS Debug)
