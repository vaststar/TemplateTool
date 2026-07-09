include(BuildModule)

set(MODULE_MODULE_NAME ViewModels)
set(MODULE_IDE_FOLDER commonhead)

# Aggregated by subdirectories below.
set(MODULE_HEADER_BASE_DIR "")
set(MODULE_HEADER_FILES "")
set(MODULE_SOURCE_PRIVATE "")
set(MODULE_BUILD_PRIVATE_DIR "")

add_subdirectory(ViewModelCommon)
add_subdirectory(MainWindowSuiteViewModel)
add_subdirectory(PageViewModels)
add_subdirectory(AppUIViewModel)
add_subdirectory(ClientInfoViewModel)
add_subdirectory(UpgradeViewModel)
add_subdirectory(MediaCameraViewModel)
add_subdirectory(CameraDirectoryViewModel)
add_subdirectory(InvocationViewModel)

set(MODULE_DEPENDENCIES_PUBLIC
    CommonHeadCommonFile
    VMNotificationHelper
    ResourceLoader
)
set(MODULE_DEPENDENCIES_PRIVATE
    CommonHeadFramework
    ServiceLocator
    ContactService
    MiniAppService
    MiniAppRuntimeAgent
    CameraDirectoryService
    MediaService
    ImageService
    ClientInfoService
    InvocationService
    ViewModelUtils
    StabilityService
    PerformanceService
    FeatureSettingsService
    UpgradeService
    Base64Utils
    JsonUtils
    TimeUtils
    UUIDUtils
    SystemUtils
    NetworkProxyAgent
    ScreenCaptureUtils
    ScreenRecordingUtils
    ScreenRecordingAgent
    ImageProcessUtils
    CompressionUtils
    ThreadPoolUtils
    FilePathUtils
    magic_enum
)

set(MODULE_BUILD_INTERFACE_DIR ${MODULE_HEADER_BASE_DIR})
set(MODULE_INSTALL_INTERFACE_DIR include)

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
TARGET_DEFINITIONS ${ADD_COMMONHEAD_DEFINITIONS}
IDE_FOLDER ${MODULE_IDE_FOLDER}
)
