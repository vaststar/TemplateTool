list(APPEND MODULE_SOURCE_PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include/ViewModelSignalEmitter/RegisterViewModelMetaTypes.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/ViewModelSignalEmitter/MediaCameraViewModelEmitter.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/ViewModelSignalEmitter/CameraDirectoryViewModelEmitter.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/ViewModelSignalEmitter/ContactListViewModelEmitter.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/ViewModelSignalEmitter/MiniAppListViewModelEmitter.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/ViewModelSignalEmitter/MiniAppRuntimeViewModelEmitter.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/ViewModelSignalEmitter/InvocationViewModelEmitter.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/ViewModelSignalEmitter/AppUIViewModelEmitter.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/ViewModelSignalEmitter/MainWindowViewModelEmitter.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/ViewModelSignalEmitter/StabilityViewModelEmitter.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/ViewModelSignalEmitter/SideBarViewModelEmitter.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/ViewModelSignalEmitter/ToolsViewModelEmitter.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/ViewModelSignalEmitter/SettingsViewModelEmitter.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/ViewModelSignalEmitter/NetworkProxyViewModelEmitter.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/ViewModelSignalEmitter/ScreenshotViewModelEmitter.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/ViewModelSignalEmitter/RecordingViewModelEmitter.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/ViewModelSignalEmitter/UpgradeViewModelEmitter.h
)
set(MODULE_SOURCE_PRIVATE ${MODULE_SOURCE_PRIVATE} PARENT_SCOPE)

list(APPEND MODULE_PRIVATE_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/include ${CMAKE_CURRENT_SOURCE_DIR}/include/ViewModelSignalEmitter)
set(MODULE_PRIVATE_INCLUDE_DIRS ${MODULE_PRIVATE_INCLUDE_DIRS} PARENT_SCOPE)
