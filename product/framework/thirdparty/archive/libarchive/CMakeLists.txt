# =============================================================================
# Archive Library - libarchive (Prebuilt)
# =============================================================================
# Multi-format archive backend for .tar/.tar.gz and more.
# Built via: .github/workflows/build-libarchive-all-platforms.yml
# =============================================================================

if(WIN32)
    add_subdirectory(libarchive/windows)
elseif(APPLE)
    if(CMAKE_SYSTEM_NAME STREQUAL "iOS")
        add_subdirectory(libarchive/ios)
    else()
        add_subdirectory(libarchive/macosx)
    endif()
elseif(ANDROID)
    add_subdirectory(libarchive/android)
elseif(UNIX)
    add_subdirectory(libarchive/linux)
else()
    message(FATAL_ERROR "[libarchive] Unsupported platform: ${CMAKE_SYSTEM_NAME}")
endif()
