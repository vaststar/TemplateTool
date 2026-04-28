list(APPEND MODULE_SOURCE_PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/RegisterViewModelMetaTypes.h
    ${CMAKE_CURRENT_SOURCE_DIR}/MediaCameraViewModelEmitter.h
    ${CMAKE_CURRENT_SOURCE_DIR}/InvocationViewModelEmitter.h
    ${CMAKE_CURRENT_SOURCE_DIR}/AppUIViewModelEmitter.h
    ${CMAKE_CURRENT_SOURCE_DIR}/MainWindowViewModelEmitter.h
    ${CMAKE_CURRENT_SOURCE_DIR}/StabilityViewModelEmitter.h
    ${CMAKE_CURRENT_SOURCE_DIR}/SideBarViewModelEmitter.h
    ${CMAKE_CURRENT_SOURCE_DIR}/ToolsViewModelEmitter.h
    ${CMAKE_CURRENT_SOURCE_DIR}/SettingsViewModelEmitter.h
    ${CMAKE_CURRENT_SOURCE_DIR}/NetworkProxyViewModelEmitter.h
    ${CMAKE_CURRENT_SOURCE_DIR}/ScreenshotViewModelEmitter.h
    ${CMAKE_CURRENT_SOURCE_DIR}/RecordingViewModelEmitter.h
    ${CMAKE_CURRENT_SOURCE_DIR}/UpgradeViewModelEmitter.h
)
set(MODULE_SOURCE_PRIVATE ${MODULE_SOURCE_PRIVATE} PARENT_SCOPE)
