include(BuildInterface)

set(MODULE_HEADER_BASE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)

BuildInterface(
    MODULE_NAME StateMachineUtils
    IDE_FOLDER framework/Utilities
    TARGET_SOURCE_HEADER_BASE_DIR ${MODULE_HEADER_BASE_DIR}
    TARGET_SOURCE_PUBLIC_HEADER
        ${MODULE_HEADER_BASE_DIR}/ucf/Utilities/StateMachineUtils/StateMachine.h
        ${MODULE_HEADER_BASE_DIR}/ucf/Utilities/StateMachineUtils/detail/Log.h
        ${MODULE_HEADER_BASE_DIR}/ucf/Utilities/StateMachineUtils/detail/MoveOnlyFunction.h
        ${MODULE_HEADER_BASE_DIR}/ucf/Utilities/StateMachineUtils/detail/Traits.h
        ${MODULE_HEADER_BASE_DIR}/ucf/Utilities/StateMachineUtils/detail/Transitions.h
    TARGET_INCLUDE_DIRECTORIES_BUILD_INTERFACE ${MODULE_HEADER_BASE_DIR}
    TARGET_INCLUDE_DIRECTORIES_INSTALL_INTERFACE include
)

add_subdirectory(unit_test)
