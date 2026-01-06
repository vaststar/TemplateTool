include_guard()
function(detect_platform OUT_VAR)
    # -----------------------------------------------
    # 平台/架构组合 (Platform/Architecture Combinations)
    # -----------------------------------------------
    # Windows       : WINDOWS_X86, WINDOWS_X64, WINDOWS_ARM64
    # macOS         : MAC_INTEL, MAC_ARM
    # Linux         : LINUX_X64, LINUX_ARM
    # Android       : ANDROID_ARM, ANDROID_ARM64, ANDROID_X86, ANDROID_X64
    # iOS           : IOS_ARM
    # tvOS          : TVOS_ARM
    # watchOS       : WATCHOS_ARM
    # QNX           : QNX
    # VISION        : VISION
    # VxWorks       : VXWORKS
    # HarmonyOS     : HARMONYOS_ARM, HARMONYOS_ARM64
    # Emscripten    : WASM
    # Wasi           : WASI
    # 未知平台        : UNKNOWN_PLATFORM
    # -----------------------------------------------

    set(_platform "UNKNOWN_PLATFORM")

    # Windows
    if (CMAKE_SYSTEM_NAME STREQUAL "Windows")
        if (CMAKE_GENERATOR_PLATFORM STREQUAL "Win32")
            set(_platform "WINDOWS_X86")
        elseif (CMAKE_GENERATOR_PLATFORM STREQUAL "x64")
            set(_platform "WINDOWS_X64")
        elseif (CMAKE_GENERATOR_PLATFORM STREQUAL "ARM64")
            set(_platform "WINDOWS_ARM64")
        else()
            if (CMAKE_SIZEOF_VOID_P EQUAL 8)
                set(_platform "WINDOWS_X64")
            else()
                set(_platform "WINDOWS_X86")
            endif()
        endif()

    # Apple 平台 (macOS, iOS, tvOS, watchOS)
    elseif (CMAKE_SYSTEM_NAME STREQUAL "Darwin")
        # 先检测特定的操作系统类型
        if (CMAKE_SYSTEM_NAME STREQUAL "iOS")
            set(_platform "IOS_ARM")
        elseif (CMAKE_SYSTEM_NAME STREQUAL "tvOS")
            set(_platform "TVOS_ARM")
        elseif (CMAKE_SYSTEM_NAME STREQUAL "watchOS")
            set(_platform "WATCHOS_ARM")
        # 再检测 macOS 架构
        elseif (CMAKE_OSX_ARCHITECTURES STREQUAL "x86_64")
            set(_platform "MAC_INTEL")
        elseif (CMAKE_OSX_ARCHITECTURES STREQUAL "arm64")
            set(_platform "MAC_ARM")
        elseif (CMAKE_SYSTEM_PROCESSOR MATCHES "arm64|aarch64")
            set(_platform "MAC_ARM")
        elseif (CMAKE_SYSTEM_PROCESSOR MATCHES "x86_64")
            set(_platform "MAC_INTEL")
        else()
            set(_platform "APPLE_UNKNOWN")
        endif()

    # Linux
    elseif (CMAKE_SYSTEM_NAME STREQUAL "Linux")
        if (CMAKE_HOST_SYSTEM_PROCESSOR STREQUAL "x86_64")
            set(_platform "LINUX_X64")
        elseif (CMAKE_HOST_SYSTEM_PROCESSOR STREQUAL "aarch64")
            set(_platform "LINUX_ARM")
        else()
            set(_platform "LINUX_UNKNOWN")
        endif()

    # Android
    elseif (CMAKE_SYSTEM_NAME STREQUAL "Android")
        if (CMAKE_ANDROID_ARCH_ABI STREQUAL "armeabi-v7a")
            set(_platform "ANDROID_ARM")
        elseif (CMAKE_ANDROID_ARCH_ABI STREQUAL "arm64-v8a")
            set(_platform "ANDROID_ARM64")
        elseif (CMAKE_ANDROID_ARCH_ABI STREQUAL "x86")
            set(_platform "ANDROID_X86")
        elseif (CMAKE_ANDROID_ARCH_ABI STREQUAL "x86_64")
            set(_platform "ANDROID_X64")
        else()
            set(_platform "ANDROID_UNKNOWN")
        endif()

    # 专用平台 (QNX / VISION / VxWorks / HarmonyOS)
    elseif (CMAKE_SYSTEM_NAME STREQUAL "QNX")
        set(_platform "QNX")
    elseif (CMAKE_SYSTEM_NAME STREQUAL "VISION")
        set(_platform "VISION")
    elseif (CMAKE_SYSTEM_NAME STREQUAL "VxWorks")
        set(_platform "VXWORKS")
    elseif (CMAKE_SYSTEM_NAME STREQUAL "HarmonyOS")
        if (CMAKE_HOST_SYSTEM_PROCESSOR STREQUAL "aarch64")
            set(_platform "HARMONYOS_ARM64")
        elseif (CMAKE_HOST_SYSTEM_PROCESSOR STREQUAL "arm")
            set(_platform "HARMONYOS_ARM")
        else()
            set(_platform "HARMONYOS_UNKNOWN")
        endif()

    # WebAssembly (Emscripten / WASI)
    elseif (CMAKE_SYSTEM_NAME STREQUAL "Emscripten")
        set(_platform "WASM")
    elseif (CMAKE_SYSTEM_NAME STREQUAL "Wasi")
        set(_platform "WASI")

    # 未知平台
    else()
        set(_platform "UNKNOWN_PLATFORM")
    endif()

    # 设置返回值
    set(${OUT_VAR} ${_platform} PARENT_SCOPE)
endfunction()
