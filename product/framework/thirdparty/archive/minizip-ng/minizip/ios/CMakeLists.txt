# =============================================================================
# minizip-ng - iOS Platform (Prebuilt XCFramework)
# =============================================================================
# Prebuilt with DEFLATE compression (zlib statically linked)
# Static library only for iOS
# =============================================================================

set(MZ_ROOT "${CMAKE_CURRENT_SOURCE_DIR}")

# Check for XCFramework
if(EXISTS "${MZ_ROOT}/libminizip.xcframework")
    set(MZ_XCFRAMEWORK "${MZ_ROOT}/libminizip.xcframework")
    set(MZ_INCLUDE "${MZ_ROOT}/include")
    
    message(STATUS "[minizip] iOS XCFramework: ${MZ_XCFRAMEWORK}")
    
    # Create interface library for XCFramework
    add_library(minizip INTERFACE)
    target_include_directories(minizip INTERFACE "${MZ_INCLUDE}")
    
    # Link XCFramework
    target_link_libraries(minizip INTERFACE "${MZ_XCFRAMEWORK}")
    
else()
    # Fallback to individual platform builds
    if(CMAKE_OSX_SYSROOT MATCHES "iphoneos")
        set(MZ_PLATFORM_DIR "iphoneos-arm64")
    elseif(CMAKE_OSX_ARCHITECTURES MATCHES "arm64")
        set(MZ_PLATFORM_DIR "iphonesimulator-arm64")
    else()
        set(MZ_PLATFORM_DIR "iphonesimulator-x86_64")
    endif()
    
    set(MZ_LIB_ROOT "${MZ_ROOT}/${MZ_PLATFORM_DIR}")
    
    if(NOT EXISTS "${MZ_LIB_ROOT}/include/mz.h")
        message(FATAL_ERROR "[minizip] Prebuilt not found at ${MZ_LIB_ROOT}. Run GitHub Action first.")
    endif()
    
    find_library(MZ_LIBRARY
        NAMES minizip
        PATHS "${MZ_LIB_ROOT}/lib"
        NO_DEFAULT_PATH
    )
    
    message(STATUS "[minizip] iOS ${MZ_PLATFORM_DIR}")
    
    add_library(minizip INTERFACE)
    target_include_directories(minizip INTERFACE "${MZ_LIB_ROOT}/include")
    target_link_libraries(minizip INTERFACE "${MZ_LIBRARY}")
endif()

# Alias for unified access
add_library(archive::zip ALIAS minizip)
