# macOS curl library (Universal Binary: arm64 + x86_64)
# curl version: 8.5.0
# SSL backend: Secure Transport (macOS native)

add_library(libcurl STATIC IMPORTED GLOBAL)

set_target_properties(libcurl PROPERTIES
    IMPORTED_LOCATION "${CMAKE_CURRENT_SOURCE_DIR}/lib/libcurl.a"
    INTERFACE_INCLUDE_DIRECTORIES "${CMAKE_CURRENT_SOURCE_DIR}/include"
)

# Link required macOS frameworks for Secure Transport SSL
target_link_libraries(libcurl INTERFACE
    "-framework Security"
    "-framework CoreFoundation"
    "-framework SystemConfiguration"
)
