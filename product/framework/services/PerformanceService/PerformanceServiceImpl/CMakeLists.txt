include(BuildModule)

# PerformanceServiceImpl (SHARED) - concrete implementation with platform monitors.
set(MODULE_MODULE_NAME PerformanceServiceImpl)
set(MODULE_IDE_FOLDER framework/Services)
set(MODULE_HEADER_BASE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)

set(MODULE_HEADER_FILES
    include/ucf/Services/PerformanceService/PerformanceServiceImplExport.h
    include/ucf/Services/PerformanceService/PerformanceServiceCreator.h
)

set(MODULE_SOURCE_COMMON
    src/PerformanceServiceLogger.h
    src/PerformanceService.h
    src/PerformanceService.cpp
    src/PerformanceNotificationSink.h
    src/PerformanceManager.h
    src/PerformanceManager.cpp
    src/TimingTracker.h
    src/TimingTracker.cpp
    src/IMemoryMonitor.h
    src/IMemoryMonitor.cpp
    src/ICPUMonitor.h
    src/ICPUMonitor.cpp
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
    PerformanceServiceApi
)
set(MODULE_DEPENDENCIES_PRIVATE
    CoreFrameworkApi
    LoggerUtils
    JsonUtils
    SinkNotifier
)
set(MODULE_BUILD_INTERFACE_DIR ${MODULE_HEADER_BASE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/src)
set(MODULE_INSTALL_INTERFACE_DIR include)
set(MODULE_BUILD_PRIVATE_DIR src)

set(MODULE_PUBLIC_DEFINITIONS)
set(MODULE_PRIVATE_DEFINITIONS)
if (SERVICE_LIB_TYPE STREQUAL "STATIC_LIB")
    set(MODULE_PUBLIC_DEFINITIONS PERFORMANCE_SERVICE_IMPL_STATIC)
else()
    set(MODULE_PRIVATE_DEFINITIONS PERFORMANCE_SERVICE_IMPL_SHARED)
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

