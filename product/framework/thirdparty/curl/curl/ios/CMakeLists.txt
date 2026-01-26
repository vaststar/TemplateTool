# iOS curl library
# curl version: 8.5.0
# SSL backend: Secure Transport (iOS native)
# Device: arm64
# Simulator: arm64 + x86_64 (Universal Binary)

add_library(libcurl STATIC IMPORTED GLOBAL)

# Determine which library to use based on SDK
if(CMAKE_OSX_SYSROOT MATCHES ".*iPhoneSimulator.*" OR CMAKE_OSX_SYSROOT MATCHES ".*iphonesimulator.*")
    set(CURL_LIB_PATH "${CMAKE_CURRENT_SOURCE_DIR}/simulator/lib/libcurl.a")
else()
    set(CURL_LIB_PATH "${CMAKE_CURRENT_SOURCE_DIR}/device/lib/libcurl.a")
endif()

set_target_properties(libcurl PROPERTIES
    IMPORTED_LOCATION "${CURL_LIB_PATH}"
    INTERFACE_INCLUDE_DIRECTORIES "${CMAKE_CURRENT_SOURCE_DIR}/include"
)

# Link required iOS frameworks for Secure Transport SSL
target_link_libraries(libcurl INTERFACE
    "-framework Security"
    "-framework CoreFoundation"
    "-framework SystemConfiguration"
)
