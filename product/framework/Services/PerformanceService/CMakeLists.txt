include(BuildModule)

set(MODULE_MODULE_NAME PerformanceService)
set(MODULE_IDE_FOLDER framework/Services)

set(MODULE_HEADER_BASE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)

set(MODULE_HEADER_FILES #for include and install header
    include/ucf/Services/PerformanceService/IPerformanceServiceCallback.h
    include/ucf/Services/PerformanceService/IPerformanceService.h
    include/ucf/Services/PerformanceService/PerformanceInfo.h
)

set(MODULE_SOURCE_COMMON
    src/PerformanceServiceLogger.h
    src/PerformanceService.h
    src/PerformanceService.cpp
    src/PerformanceManager.h
    src/PerformanceManager.cpp
    src/TimingTracker.h
    src/TimingTracker.cpp
    src/platform/IMemoryMonitor.h
    src/platform/IMemoryMonitor.cpp
    src/platform/ICPUMonitor.h
    src/platform/ICPUMonitor.cpp
)

if(APPLE)
    set(MODULE_SOURCE_PLATFORM
        src/platform/MacOSMemoryMonitor.h
        src/platform/MacOSMemoryMonitor.cpp
        src/platform/MacOSCPUMonitor.h
        src/platform/MacOSCPUMonitor.cpp
    )
elseif(UNIX AND NOT APPLE)
    set(MODULE_SOURCE_PLATFORM
        src/platform/LinuxMemoryMonitor.h
        src/platform/LinuxMemoryMonitor.cpp
        src/platform/LinuxCPUMonitor.h
        src/platform/LinuxCPUMonitor.cpp
    )
elseif(WIN32)
    set(MODULE_SOURCE_PLATFORM
        src/platform/WindowsMemoryMonitor.h
        src/platform/WindowsMemoryMonitor.cpp
        src/platform/WindowsCPUMonitor.h
        src/platform/WindowsCPUMonitor.cpp
    )
endif()

set(MODULE_SOURCE_PRIVATE
    ${MODULE_SOURCE_COMMON}
    ${MODULE_SOURCE_PLATFORM}
)

set(MODULE_DEPENDENCIES_PUBLIC
    ServiceExportMacro
    ServiceDeclaration
    NotificationHelper
)

set(MODULE_DEPENDENCIES_PRIVATE
    CoreFramework
    LoggerUtils
    nlohmann_json::nlohmann_json
)

set(MODULE_BUILD_INTERFACE_DIR ${MODULE_HEADER_BASE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/src)
set(MODULE_INSTALL_INTERFACE_DIR include)
set(MODULE_BUILD_PRIVATE_DIR src)

BuildModule(
MODULE_NAME ${MODULE_MODULE_NAME}
TARGET_SOURCE_PRIVATE ${MODULE_SOURCE_PRIVATE}
TARGET_SOURCE_HEADER_BASE_DIR ${MODULE_HEADER_BASE_DIR}
TARGET_SOURCE_PUBLIC_HEADER ${MODULE_HEADER_FILES}
TARGET_ADD_LINK_LIBRARY_PRIVATE ${MODULE_DEPENDENCIES_PRIVATE}
TARGET_ADD_LINK_LIBRARY_PUBLIC ${MODULE_DEPENDENCIES_PUBLIC}
TARGET_INCLUDE_DIRECTORIES_BUILD_INTERFACE  ${MODULE_BUILD_INTERFACE_DIR}
TARGET_INCLUDE_DIRECTORIES_INSTALL_INTERFACE ${MODULE_INSTALL_INTERFACE_DIR}
TARGET_INCLUDE_DIRECTORIES_PRIVATE ${MODULE_BUILD_PRIVATE_DIR}
TARGET_DEFINITIONS ${ADD_SERVICE_DEFINITIONS}
IDE_FOLDER ${MODULE_IDE_FOLDER}
)

add_subdirectory(test)
