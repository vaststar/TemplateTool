include(BuildModule)

set(MODULE_MODULE_NAME StabilityService)
set(MODULE_IDE_FOLDER framework/services)

set(MODULE_HEADER_BASE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)

set(MODULE_HEADER_FILES
    include/ucf/Services/StabilityService/IStabilityService.h
    include/ucf/Services/StabilityService/CrashInfo.h
    include/ucf/Services/StabilityService/HangInfo.h
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

# Platform-specific crash files
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
    ServiceDeclaration
)

set(MODULE_DEPENDENCIES_PRIVATE
    CoreFramework
    LoggerUtils
    FilePathUtils
    ClientInfoService
)

set(MODULE_BUILD_INTERFACE_DIR ${MODULE_HEADER_BASE_DIR})
set(MODULE_INSTALL_INTERFACE_DIR include)
set(MODULE_BUILD_PRIVATE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)

BuildModule(
    MODULE_NAME ${MODULE_MODULE_NAME}
    TARGET_SOURCE_PRIVATE ${MODULE_SOURCE_PRIVATE}
    TARGET_SOURCE_HEADER_BASE_DIR ${MODULE_HEADER_BASE_DIR}
    TARGET_SOURCE_PUBLIC_HEADER ${MODULE_HEADER_FILES}
    TARGET_ADD_LINK_LIBRARY_PRIVATE ${MODULE_DEPENDENCIES_PRIVATE}
    TARGET_ADD_LINK_LIBRARY_PUBLIC ${MODULE_DEPENDENCIES_PUBLIC}
    TARGET_INCLUDE_DIRECTORIES_BUILD_INTERFACE ${MODULE_BUILD_INTERFACE_DIR}
    TARGET_INCLUDE_DIRECTORIES_INSTALL_INTERFACE ${MODULE_INSTALL_INTERFACE_DIR}
    TARGET_INCLUDE_DIRECTORIES_PRIVATE ${MODULE_BUILD_PRIVATE_DIR}
    TARGET_DEFINITIONS ${ADD_SERVICE_DEFINITIONS}
    IDE_FOLDER ${MODULE_IDE_FOLDER}
)

add_subdirectory(test)
