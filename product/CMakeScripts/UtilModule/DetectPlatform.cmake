include_guard()

# ==========================================
# Function: detect_platform
# Detect the current build platform and architecture
# ==========================================
function(detect_platform OUT_VAR)
    # -----------------------------------------------
    # Platform/Architecture Return Values:
    # -----------------------------------------------
    # Windows       : WINDOWS_X86, WINDOWS_X64, WINDOWS_ARM64
    # macOS         : MAC_INTEL, MAC_ARM
    # Linux         : LINUX_X64, LINUX_ARM64
    # Android       : ANDROID_ARM, ANDROID_ARM64, ANDROID_X86, ANDROID_X64
    # iOS           : IOS_ARM64, IOS_SIMULATOR_X64, IOS_SIMULATOR_ARM64
    # tvOS          : TVOS_ARM64
    # watchOS       : WATCHOS_ARM64
    # visionOS      : VISIONOS_ARM64
    # QNX           : QNX
    # VxWorks       : VXWORKS
    # HarmonyOS     : HARMONYOS_ARM, HARMONYOS_ARM64
    # Emscripten    : WASM
    # WASI          : WASI
    # Unknown       : UNKNOWN_PLATFORM
    # -----------------------------------------------

    set(_platform "UNKNOWN_PLATFORM")

    # ==========================================
    # Windows
    # ==========================================
    if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
        if(CMAKE_GENERATOR_PLATFORM STREQUAL "Win32")
            set(_platform "WINDOWS_X86")
        elseif(CMAKE_GENERATOR_PLATFORM STREQUAL "x64")
            set(_platform "WINDOWS_X64")
        elseif(CMAKE_GENERATOR_PLATFORM STREQUAL "ARM64")
            set(_platform "WINDOWS_ARM64")
        else()
            # Fallback: detect by pointer size
            if(CMAKE_SIZEOF_VOID_P EQUAL 8)
                set(_platform "WINDOWS_X64")
            else()
                set(_platform "WINDOWS_X86")
            endif()
        endif()

    # ==========================================
    # Apple Embedded: iOS, tvOS, watchOS, visionOS
    # (Must check before Darwin/macOS)
    # ==========================================
    elseif(CMAKE_SYSTEM_NAME STREQUAL "iOS")
        if(CMAKE_OSX_SYSROOT MATCHES "Simulator")
            if(CMAKE_OSX_ARCHITECTURES STREQUAL "x86_64")
                set(_platform "IOS_SIMULATOR_X64")
            else()
                set(_platform "IOS_SIMULATOR_ARM64")
            endif()
        else()
            set(_platform "IOS_ARM64")
        endif()
    elseif(CMAKE_SYSTEM_NAME STREQUAL "tvOS")
        set(_platform "TVOS_ARM64")
    elseif(CMAKE_SYSTEM_NAME STREQUAL "watchOS")
        set(_platform "WATCHOS_ARM64")
    elseif(CMAKE_SYSTEM_NAME STREQUAL "visionOS")
        set(_platform "VISIONOS_ARM64")

    # ==========================================
    # macOS (Darwin)
    # ==========================================
    elseif(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
        if(CMAKE_OSX_ARCHITECTURES STREQUAL "x86_64")
            set(_platform "MAC_INTEL")
        elseif(CMAKE_OSX_ARCHITECTURES STREQUAL "arm64")
            set(_platform "MAC_ARM")
        elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "arm64|aarch64")
            set(_platform "MAC_ARM")
        elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "x86_64")
            set(_platform "MAC_INTEL")
        else()
            set(_platform "MAC_UNKNOWN")
        endif()

    # ==========================================
    # Linux
    # ==========================================
    elseif(CMAKE_SYSTEM_NAME STREQUAL "Linux")
        if(CMAKE_SYSTEM_PROCESSOR STREQUAL "x86_64")
            set(_platform "LINUX_X64")
        elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "aarch64|arm64")
            set(_platform "LINUX_ARM64")
        elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "^arm")
            set(_platform "LINUX_ARM")
        else()
            set(_platform "LINUX_UNKNOWN")
        endif()

    # ==========================================
    # Android
    # ==========================================
    elseif(CMAKE_SYSTEM_NAME STREQUAL "Android")
        if(CMAKE_ANDROID_ARCH_ABI STREQUAL "armeabi-v7a")
            set(_platform "ANDROID_ARM")
        elseif(CMAKE_ANDROID_ARCH_ABI STREQUAL "arm64-v8a")
            set(_platform "ANDROID_ARM64")
        elseif(CMAKE_ANDROID_ARCH_ABI STREQUAL "x86")
            set(_platform "ANDROID_X86")
        elseif(CMAKE_ANDROID_ARCH_ABI STREQUAL "x86_64")
            set(_platform "ANDROID_X64")
        else()
            set(_platform "ANDROID_UNKNOWN")
        endif()

    # ==========================================
    # Embedded/RTOS platforms
    # ==========================================
    elseif(CMAKE_SYSTEM_NAME STREQUAL "QNX")
        set(_platform "QNX")
    elseif(CMAKE_SYSTEM_NAME STREQUAL "VxWorks")
        set(_platform "VXWORKS")

    # ==========================================
    # HarmonyOS
    # ==========================================
    elseif(CMAKE_SYSTEM_NAME STREQUAL "OHOS" OR CMAKE_SYSTEM_NAME STREQUAL "HarmonyOS")
        if(CMAKE_SYSTEM_PROCESSOR MATCHES "aarch64|arm64")
            set(_platform "HARMONYOS_ARM64")
        elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "^arm")
            set(_platform "HARMONYOS_ARM")
        else()
            set(_platform "HARMONYOS_UNKNOWN")
        endif()

    # ==========================================
    # WebAssembly
    # ==========================================
    elseif(CMAKE_SYSTEM_NAME STREQUAL "Emscripten")
        set(_platform "WASM")
    elseif(CMAKE_SYSTEM_NAME STREQUAL "WASI")
        set(_platform "WASI")

    # ==========================================
    # Unknown platform
    # ==========================================
    else()
        set(_platform "UNKNOWN_PLATFORM")
    endif()

    # Return result
    set(${OUT_VAR} ${_platform} PARENT_SCOPE)
endfunction()
