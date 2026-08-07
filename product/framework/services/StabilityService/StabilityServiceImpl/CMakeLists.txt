include(BuildModule)

# StabilityServiceImpl (SHARED) - concrete implementation with crash/hang handlers.
set(MODULE_MODULE_NAME StabilityServiceImpl)
set(MODULE_IDE_FOLDER framework/Services)
set(MODULE_HEADER_BASE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)

set(MODULE_HEADER_FILES
    include/ucf/Services/StabilityService/StabilityServiceImplExport.h
    include/ucf/Services/StabilityService/StabilityServiceCreator.h
)

set(MODULE_SOURCE_PRIVATE
    src/StabilityService.h
    src/StabilityService.cpp
    src/StabilityServiceLogger.h
    # Crash module
    src/crash/CrashManager.h
    src/crash/CrashManager.cpp
    src/crash/IPlatformCrashHandler.h
    src/crash/IPlatformCrashHandler.cpp
    # Hang module
    src/hang/HangManager.h
    src/hang/HangManager.cpp
    src/hang/IPlatformHangHandler.h
    src/hang/IPlatformHangHandler.cpp
)

# Platform-specific crash/hang files
if(APPLE)
    list(APPEND MODULE_SOURCE_PRIVATE
        src/crash/platform/PosixCrashHandler.h
        src/crash/platform/PosixCrashHandler.cpp
        src/hang/platform/MacOSHangHandler.h
        src/hang/platform/MacOSHangHandler.cpp
    )
elseif(UNIX)
    list(APPEND MODULE_SOURCE_PRIVATE
        src/crash/platform/PosixCrashHandler.h
        src/crash/platform/PosixCrashHandler.cpp
        src/hang/platform/LinuxHangHandler.h
        src/hang/platform/LinuxHangHandler.cpp
    )
elseif(WIN32)
    list(APPEND MODULE_SOURCE_PRIVATE
        src/crash/platform/WindowsCrashHandler.h
        src/crash/platform/WindowsCrashHandler.cpp
        src/hang/platform/WindowsHangHandler.h
        src/hang/platform/WindowsHangHandler.cpp
    )
endif()

set(MODULE_DEPENDENCIES_PUBLIC
    StabilityServiceApi
)
set(MODULE_DEPENDENCIES_PRIVATE
    CoreFramework
    LoggerUtils
    FilePathUtils
    ClientInfoServiceApi
    TimeUtils
)
set(MODULE_BUILD_INTERFACE_DIR ${MODULE_HEADER_BASE_DIR})
set(MODULE_INSTALL_INTERFACE_DIR include)
set(MODULE_BUILD_PRIVATE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)

set(MODULE_PUBLIC_DEFINITIONS)
set(MODULE_PRIVATE_DEFINITIONS)
if (SERVICE_LIB_TYPE STREQUAL "STATIC_LIB")
    set(MODULE_PUBLIC_DEFINITIONS STABILITY_SERVICE_IMPL_STATIC)
else()
    set(MODULE_PRIVATE_DEFINITIONS STABILITY_SERVICE_IMPL_SHARED)
endif()

BuildModule(
    ${SERVICE_LIB_TYPE}
    MODULE_NAME ${MODULE_MODULE_NAME}
    TARGET_SOURCE_PRIVATE ${MODULE_SOURCE_PRIVATE}
    TARGET_SOURCE_HEADER_BASE_DIR ${MODULE_HEADER_BASE_DIR}
    TARGET_SOURCE_PUBLIC_HEADER ${MODULE_HEADER_FILES}
    TARGET_ADD_LINK_LIBRARY_PUBLIC ${MODULE_DEPENDENCIES_PUBLIC}
    TARGET_ADD_LINK_LIBRARY_PRIVATE ${MODULE_DEPENDENCIES_PRIVATE}
    TARGET_INCLUDE_DIRECTORIES_BUILD_INTERFACE ${MODULE_BUILD_INTERFACE_DIR}
    TARGET_INCLUDE_DIRECTORIES_INSTALL_INTERFACE ${MODULE_INSTALL_INTERFACE_DIR}
    TARGET_INCLUDE_DIRECTORIES_PRIVATE ${MODULE_BUILD_PRIVATE_DIR}
    TARGET_PUBLIC_DEFINITIONS ${MODULE_PUBLIC_DEFINITIONS}
    TARGET_PRIVATE_DEFINITIONS ${MODULE_PRIVATE_DEFINITIONS}
    IDE_FOLDER ${MODULE_IDE_FOLDER}
)

