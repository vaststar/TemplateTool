include(RuntimePayloadRegistry)

option(TT_ENABLE_RUNTIME_DEPENDENCY_AUDIT
    "Report runtime dependencies missing from the install tree"
    ON
)

function(InstallApplicationBundle)
    set(one_value_args TARGET QML_SOURCE_DIR)
    set(multi_value_args QT_DEPLOY_TARGETS PROGRAM_PAYLOADS)
    cmake_parse_arguments(
        PARSE_ARGV 0 APPLICATION_BUNDLE
        "" "${one_value_args}" "${multi_value_args}"
    )

    if(NOT APPLICATION_BUNDLE_TARGET)
        message(FATAL_ERROR
            "[InstallApplicationBundle] TARGET is required")
    endif()
    if(APPLICATION_BUNDLE_KEYWORDS_MISSING_VALUES)
        message(FATAL_ERROR
            "[InstallApplicationBundle:${APPLICATION_BUNDLE_TARGET}] "
            "Arguments missing values: "
            "${APPLICATION_BUNDLE_KEYWORDS_MISSING_VALUES}")
    endif()
    if(APPLICATION_BUNDLE_UNPARSED_ARGUMENTS)
        message(FATAL_ERROR
            "[InstallApplicationBundle:${APPLICATION_BUNDLE_TARGET}] "
            "Unknown arguments: ${APPLICATION_BUNDLE_UNPARSED_ARGUMENTS}")
    endif()
    if(NOT TARGET "${APPLICATION_BUNDLE_TARGET}")
        message(FATAL_ERROR
            "[InstallApplicationBundle] Target does not exist: "
            "${APPLICATION_BUNDLE_TARGET}")
    endif()
    if(NOT APPLICATION_BUNDLE_QML_SOURCE_DIR)
        message(FATAL_ERROR
            "[InstallApplicationBundle] QML_SOURCE_DIR is required")
    endif()
    if(NOT IS_DIRECTORY "${APPLICATION_BUNDLE_QML_SOURCE_DIR}")
        message(FATAL_ERROR
            "[InstallApplicationBundle] QML_SOURCE_DIR does not exist: "
            "${APPLICATION_BUNDLE_QML_SOURCE_DIR}")
    endif()
    if(NOT APPLICATION_BUNDLE_QT_DEPLOY_TARGETS)
        message(FATAL_ERROR
            "[InstallApplicationBundle] QT_DEPLOY_TARGETS is required")
    endif()

    get_target_property(application_target
        "${APPLICATION_BUNDLE_TARGET}" ALIASED_TARGET)
    if(NOT application_target
       OR application_target MATCHES "-NOTFOUND$")
        set(application_target "${APPLICATION_BUNDLE_TARGET}")
    endif()

    get_target_property(application_name
        "${application_target}" OUTPUT_NAME)
    if(NOT application_name OR application_name MATCHES "-NOTFOUND$")
        set(application_name "${application_target}")
    endif()

    set(qt_deploy_file_names)
    foreach(qt_deploy_target
            IN LISTS APPLICATION_BUNDLE_QT_DEPLOY_TARGETS)
        if(NOT TARGET "${qt_deploy_target}")
            message(FATAL_ERROR
                "[InstallApplicationBundle] Qt deployment target does not "
                "exist: ${qt_deploy_target}")
        endif()

        get_target_property(real_qt_deploy_target
            "${qt_deploy_target}" ALIASED_TARGET)
        if(NOT real_qt_deploy_target
           OR real_qt_deploy_target MATCHES "-NOTFOUND$")
            set(real_qt_deploy_target "${qt_deploy_target}")
        endif()

        if(NOT "${real_qt_deploy_target}"
           STREQUAL "${application_target}")
            add_dependencies(
                "${application_target}" "${real_qt_deploy_target}")
        endif()
        list(APPEND qt_deploy_file_names
            "$<TARGET_FILE_NAME:${real_qt_deploy_target}>")
    endforeach()
    list(REMOVE_DUPLICATES qt_deploy_file_names)

    set(common_install_variables "
        set(APP_NAME \"${application_name}\")
        set(APPLICATION_FILE_NAME
            \"$<TARGET_FILE_NAME:${application_target}>\")
        set(QML_SOURCE_DIR \"${APPLICATION_BUNDLE_QML_SOURCE_DIR}\")
        set(QT_DEPLOY_FILE_NAMES \"${qt_deploy_file_names}\")
        set(PROGRAM_PAYLOADS \"${APPLICATION_BUNDLE_PROGRAM_PAYLOADS}\")
        set(TEMPLATE_TOOL_BINARY_DIR \"${CMAKE_BINARY_DIR}\")
        set(BUILD_CONFIG_NAME \"\${CMAKE_INSTALL_CONFIG_NAME}\")
    ")

    if(APPLE)
        install(TARGETS "${application_target}"
            EXPORT "${application_target}Targets"
            BUNDLE DESTINATION bin
        )
        set(post_install_script
            "${CMAKE_CURRENT_FUNCTION_LIST_DIR}/post_install/macos.cmake")
        set(install_description
            "bin/${application_name}.app + post_install/macos.cmake")
    elseif(WIN32)
        install(TARGETS "${application_target}"
            EXPORT "${application_target}Targets"
            RUNTIME DESTINATION bin
        )
        set(post_install_script
            "${CMAKE_CURRENT_FUNCTION_LIST_DIR}/post_install/windows.cmake")
        set(install_description "bin/ + post_install/windows.cmake")
    else()
        install(TARGETS "${application_target}"
            EXPORT "${application_target}Targets"
            RUNTIME DESTINATION bin
        )
        set(post_install_script
            "${CMAKE_CURRENT_FUNCTION_LIST_DIR}/post_install/linux.cmake")
        set(install_description "bin/ + post_install/linux.cmake")
    endif()

    # Runtime payload producers own their names and sources. This layer only
    # maps the registered payloads into the platform-specific application tree.
    InstallRegisteredRuntimePayloads(
        TARGET "${application_target}"
    )

    install(CODE "${common_install_variables}")
    install(SCRIPT "${post_install_script}")
    if(TT_ENABLE_RUNTIME_DEPENDENCY_AUDIT)
        install(SCRIPT
            "${CMAKE_CURRENT_FUNCTION_LIST_DIR}/audit/runtime_dependencies.cmake")
    endif()
    message(STATUS
        "[InstallApplicationBundle:${APPLICATION_BUNDLE_TARGET}] "
        "${install_description}")
endfunction()

InstallApplicationBundle(
    TARGET "${TT_APPLICATION_TARGET}"
    QML_SOURCE_DIR "${TT_PACKAGING_QML_SOURCE_DIR}"
    QT_DEPLOY_TARGETS ${TT_PACKAGING_QT_DEPLOY_TARGETS}
    PROGRAM_PAYLOADS ${TT_PACKAGING_PROGRAM_PAYLOADS}
)
