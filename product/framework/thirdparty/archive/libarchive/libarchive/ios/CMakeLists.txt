# =============================================================================
# libarchive - iOS Platform (Prebuilt XCFramework)
# =============================================================================

set(LA_ROOT "${CMAKE_CURRENT_SOURCE_DIR}")

if(EXISTS "${LA_ROOT}/libarchive.xcframework")
    set(LA_XCFRAMEWORK "${LA_ROOT}/libarchive.xcframework")
    set(LA_INCLUDE "${LA_ROOT}/include")

    message(STATUS "[libarchive] iOS XCFramework: ${LA_XCFRAMEWORK}")

    add_library(libarchive_prebuilt INTERFACE)
    target_include_directories(libarchive_prebuilt INTERFACE "${LA_INCLUDE}")
    target_link_libraries(libarchive_prebuilt INTERFACE "${LA_XCFRAMEWORK}")
else()
    if(CMAKE_OSX_SYSROOT MATCHES "iphoneos")
        set(LA_PLATFORM_DIR "iphoneos-arm64")
    elseif(CMAKE_OSX_ARCHITECTURES MATCHES "arm64")
        set(LA_PLATFORM_DIR "iphonesimulator-arm64")
    else()
        set(LA_PLATFORM_DIR "iphonesimulator-x86_64")
    endif()

    set(LA_LIB_ROOT "${LA_ROOT}/${LA_PLATFORM_DIR}")

    if(NOT EXISTS "${LA_LIB_ROOT}/include/archive.h")
        message(FATAL_ERROR "[libarchive] Prebuilt not found at ${LA_LIB_ROOT}. Run GitHub Action 'build-libarchive-all-platforms.yml' first.")
    endif()

    find_library(LA_LIBRARY
        NAMES archive libarchive
        PATHS "${LA_LIB_ROOT}/lib"
        NO_DEFAULT_PATH
    )

    message(STATUS "[libarchive] iOS ${LA_PLATFORM_DIR}")

    add_library(libarchive_prebuilt INTERFACE)
    target_include_directories(libarchive_prebuilt INTERFACE "${LA_LIB_ROOT}/include")
    target_link_libraries(libarchive_prebuilt INTERFACE "${LA_LIBRARY}")
endif()

add_library(archive::multi ALIAS libarchive_prebuilt)