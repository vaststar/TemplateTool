list(APPEND MODULE_SOURCE_PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include/ViewModelSingalEmitter/RegisterViewModelMetaTypes.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/ViewModelSingalEmitter/MediaCameraViewModelEmitter.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/ViewModelSingalEmitter/CameraDirectoryViewModelEmitter.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/ViewModelSingalEmitter/ContactListViewModelEmitter.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/ViewModelSingalEmitter/MiniAppListViewModelEmitter.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/ViewModelSingalEmitter/InvocationViewModelEmitter.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/ViewModelSingalEmitter/AppUIViewModelEmitter.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/ViewModelSingalEmitter/MainWindowViewModelEmitter.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/ViewModelSingalEmitter/StabilityViewModelEmitter.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/ViewModelSingalEmitter/SideBarViewModelEmitter.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/ViewModelSingalEmitter/ToolsViewModelEmitter.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/ViewModelSingalEmitter/SettingsViewModelEmitter.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/ViewModelSingalEmitter/NetworkProxyViewModelEmitter.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/ViewModelSingalEmitter/ScreenshotViewModelEmitter.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/ViewModelSingalEmitter/RecordingViewModelEmitter.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/ViewModelSingalEmitter/UpgradeViewModelEmitter.h
)
set(MODULE_SOURCE_PRIVATE ${MODULE_SOURCE_PRIVATE} PARENT_SCOPE)

list(APPEND MODULE_PRIVATE_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/include ${CMAKE_CURRENT_SOURCE_DIR}/include/ViewModelSingalEmitter)
set(MODULE_PRIVATE_INCLUDE_DIRS ${MODULE_PRIVATE_INCLUDE_DIRS} PARENT_SCOPE)
