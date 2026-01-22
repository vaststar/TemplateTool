include_guard()

# ==========================================
# Script mode: Generate version meta JSON
# ==========================================
if(CMAKE_SCRIPT_MODE_FILE)
    if(NOT EXISTS "${INPUT_META_JSON}")
        message(FATAL_ERROR "[GenerateAppVersionMeta] Version config file not found: ${INPUT_META_JSON}")
    endif()

    if(NOT EXISTS "${GIT_INFO_FILE}")
        message(FATAL_ERROR "[GenerateAppVersionMeta] Git info file not found: ${GIT_INFO_FILE}")
    endif()

    # Read Git information
    file(READ "${GIT_INFO_FILE}" GIT_INFO_CONTENT)
    string(JSON GIT_COMMIT_HASH GET "${GIT_INFO_CONTENT}" "hash")
    string(JSON GIT_COMMIT_BRANCH GET "${GIT_INFO_CONTENT}" "branch")
    string(JSON VERSION_BUILD GET "${GIT_INFO_CONTENT}" "depth")

    # Read version metadata
    file(READ "${INPUT_META_JSON}" JSON_CONTENT)
    
    # Parse version info
    string(JSON VERSION_MAJOR GET "${JSON_CONTENT}" "version" "major")
    string(JSON VERSION_MINOR GET "${JSON_CONTENT}" "version" "minor")
    string(JSON VERSION_PATCH GET "${JSON_CONTENT}" "version" "patch")
    
    # Parse company info
    string(JSON COMPANY_NAME GET "${JSON_CONTENT}" "company" "name")
    string(JSON COPYRIGHT GET "${JSON_CONTENT}" "company" "copyright")
    
    # Parse product info
    string(JSON PRODUCT_NAME GET "${JSON_CONTENT}" "product" "name")
    string(JSON PRODUCT_DESCRIPTION GET "${JSON_CONTENT}" "product" "description")

    configure_file(${INPUT_TEMPLATE} ${OUTPUT_FILE} @ONLY)
    message(STATUS "[GenerateAppVersionMeta] Generated ${OUTPUT_FILE}")
endif()

# ==========================================
# Function: generate_app_version_meta
# ==========================================
function(generate_app_version_meta)
    # Include here instead of top level to avoid script mode conflict
    include("${CMAKE_CURRENT_FUNCTION_LIST_DIR}/GenerateGitInfoMeta.cmake")
    
    set(oneValueArgs INPUT_META_JSON INPUT_VERSION_TEMPLATE OUTPUT_FILE OUTPUT_TARGET_VAR)
    cmake_parse_arguments(ARG "" "${oneValueArgs}" "" ${ARGN})

    # Validate required arguments
    if(NOT ARG_OUTPUT_FILE)
        message(FATAL_ERROR "[GenerateAppVersionMeta] OUTPUT_FILE is required")
    endif()

    if(NOT ARG_INPUT_VERSION_TEMPLATE)
        message(FATAL_ERROR "[GenerateAppVersionMeta] INPUT_VERSION_TEMPLATE is required")
    endif()
    
    if(NOT ARG_INPUT_META_JSON)
        message(FATAL_ERROR "[GenerateAppVersionMeta] INPUT_META_JSON is required")
    endif()

    # Generate unique target name based on output file path hash
    string(MD5 PATH_HASH "${ARG_OUTPUT_FILE}")
    string(SUBSTRING "${PATH_HASH}" 0 8 SHORT_HASH)
    get_filename_component(OUTPUT_NAME "${ARG_OUTPUT_FILE}" NAME_WE)
    set(TARGET_NAME "generate_${OUTPUT_NAME}_${SHORT_HASH}")

    set(GIT_INFO_FILE "${CMAKE_CURRENT_BINARY_DIR}/git_info.meta")

    message(STATUS "")
    message(STATUS "============================================================")
    message(STATUS "[GenerateAppVersionMeta] Configuring version meta generation")
    message(STATUS "============================================================")
    message(STATUS "  Output File  : ${ARG_OUTPUT_FILE}")
    message(STATUS "  Template     : ${ARG_INPUT_VERSION_TEMPLATE}")
    message(STATUS "  Meta JSON    : ${ARG_INPUT_META_JSON}")
    message(STATUS "  Git Info     : ${GIT_INFO_FILE}")
    message(STATUS "  Target Name  : ${TARGET_NAME}")
    message(STATUS "------------------------------------------------------------")
    
    # Generate git info meta first
    generate_git_info_meta(OUTPUT_FILE "${GIT_INFO_FILE}")
    
    # Add custom command to generate version meta
    add_custom_command(
        OUTPUT ${ARG_OUTPUT_FILE}
        COMMAND ${CMAKE_COMMAND} 
            -DINPUT_META_JSON=${ARG_INPUT_META_JSON}
            -DINPUT_TEMPLATE=${ARG_INPUT_VERSION_TEMPLATE}
            -DOUTPUT_FILE=${ARG_OUTPUT_FILE}
            -DGIT_INFO_FILE=${GIT_INFO_FILE}
            -P "${CMAKE_CURRENT_FUNCTION_LIST_DIR}/GenerateAppVersionMeta.cmake"
        DEPENDS ${ARG_INPUT_META_JSON} ${ARG_INPUT_VERSION_TEMPLATE} ${GIT_INFO_FILE}
        COMMENT "[GenerateAppVersionMeta] Generating ${ARG_OUTPUT_FILE}"
    )

    # Create custom target
    add_custom_target(${TARGET_NAME} ALL
        DEPENDS ${ARG_OUTPUT_FILE}
    )
    set_target_properties(${TARGET_NAME} PROPERTIES FOLDER codegen)
    
    # Return target name if output variable specified
    if(ARG_OUTPUT_TARGET_VAR)
        set(${ARG_OUTPUT_TARGET_VAR} ${TARGET_NAME} PARENT_SCOPE)
    endif()
endfunction()