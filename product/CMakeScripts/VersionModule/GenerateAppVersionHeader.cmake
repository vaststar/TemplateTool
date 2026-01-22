include_guard()

# ==========================================
# Script mode: Generate version header file
# ==========================================
if(CMAKE_SCRIPT_MODE_FILE)
    if(NOT DEFINED INPUT_JSON_FILE)
        message(FATAL_ERROR "[GenerateAppVersionHeader] INPUT_JSON_FILE variable not defined")
    endif()
    if(NOT EXISTS "${INPUT_JSON_FILE}")
        message(FATAL_ERROR "[GenerateAppVersionHeader] Input JSON file not found: ${INPUT_JSON_FILE}")
    endif()
    if(NOT DEFINED INPUT_TEMPLATE_FILE)
        message(FATAL_ERROR "[GenerateAppVersionHeader] INPUT_TEMPLATE_FILE variable not defined")
    endif()
    if(NOT DEFINED OUTPUT_H)
        message(FATAL_ERROR "[GenerateAppVersionHeader] OUTPUT_H variable not defined")
    endif()

    file(READ "${INPUT_JSON_FILE}" json_content)
    
    # Parse version info
    string(JSON VERSION_MAJOR GET "${json_content}" "VERSION" "VERSION_MAJOR")
    string(JSON VERSION_MINOR GET "${json_content}" "VERSION" "VERSION_MINOR")
    string(JSON VERSION_PATCH GET "${json_content}" "VERSION" "VERSION_PATCH")
    string(JSON VERSION_BUILD GET "${json_content}" "VERSION" "VERSION_BUILD")
    
    # Parse compilation info
    string(JSON GIT_COMMIT_HASH GET "${json_content}" "COMPILATION" "GIT_COMMIT_HASH")
    string(JSON GIT_COMMIT_BRANCH GET "${json_content}" "COMPILATION" "GIT_COMMIT_BRANCH")
    
    # Parse company info
    string(JSON COMPANY_NAME GET "${json_content}" "COMPANY" "NAME")
    string(JSON COPYRIGHT GET "${json_content}" "COMPANY" "COPYRIGHT")
    
    # Parse product info
    string(JSON PRODUCT_NAME GET "${json_content}" "PRODUCT" "NAME")
    string(JSON PRODUCT_DESCRIPTION GET "${json_content}" "PRODUCT" "DESCRIPTION")

    configure_file(${INPUT_TEMPLATE_FILE} ${OUTPUT_H} @ONLY)
    message(STATUS "[GenerateAppVersionHeader] Generated ${OUTPUT_H}")
endif()

# ==========================================
# Function: generate_app_version_header
# ==========================================
function(generate_app_version_header)
    set(oneValueArgs INPUT_JSON_FILE INPUT_JSON_TARGET INPUT_VERSION_TEMPLATE OUTPUT_FILE OUTPUT_TARGET_VAR)
    cmake_parse_arguments(ARG "" "${oneValueArgs}" "" ${ARGN})

    # Validate required arguments
    if(NOT ARG_INPUT_VERSION_TEMPLATE)
        message(FATAL_ERROR "[GenerateAppVersionHeader] INPUT_VERSION_TEMPLATE is required")
    endif()
    if(NOT ARG_INPUT_JSON_TARGET)
        message(FATAL_ERROR "[GenerateAppVersionHeader] INPUT_JSON_TARGET is required")
    endif()
    if(NOT ARG_OUTPUT_FILE)
        message(FATAL_ERROR "[GenerateAppVersionHeader] OUTPUT_FILE is required")
    endif()
    if(NOT ARG_INPUT_JSON_FILE)
        message(FATAL_ERROR "[GenerateAppVersionHeader] INPUT_JSON_FILE is required")
    endif()

    # Generate unique target name based on output file path hash
    string(MD5 PATH_HASH "${ARG_OUTPUT_FILE}")
    string(SUBSTRING "${PATH_HASH}" 0 8 SHORT_HASH)
    get_filename_component(OUTPUT_NAME "${ARG_OUTPUT_FILE}" NAME_WE)
    set(TARGET_NAME "generate_${OUTPUT_NAME}_${SHORT_HASH}")
    
    message(STATUS "")
    message(STATUS "============================================================")
    message(STATUS "[GenerateAppVersionHeader] Configuring version header generation")
    message(STATUS "============================================================")
    message(STATUS "  Output File  : ${ARG_OUTPUT_FILE}")
    message(STATUS "  Template     : ${ARG_INPUT_VERSION_TEMPLATE}")
    message(STATUS "  Input JSON   : ${ARG_INPUT_JSON_FILE}")
    message(STATUS "  JSON Target  : ${ARG_INPUT_JSON_TARGET}")
    message(STATUS "  Target Name  : ${TARGET_NAME}")
    message(STATUS "------------------------------------------------------------")
   
    # Add custom command to generate version header
    add_custom_command(
        OUTPUT ${ARG_OUTPUT_FILE}
        COMMAND ${CMAKE_COMMAND} 
            -DINPUT_JSON_FILE=${ARG_INPUT_JSON_FILE}
            -DINPUT_TEMPLATE_FILE=${ARG_INPUT_VERSION_TEMPLATE}
            -DOUTPUT_H=${ARG_OUTPUT_FILE}
            -P "${CMAKE_CURRENT_FUNCTION_LIST_DIR}/GenerateAppVersionHeader.cmake"
        DEPENDS ${ARG_INPUT_JSON_FILE} ${ARG_INPUT_VERSION_TEMPLATE}
        COMMENT "[GenerateAppVersionHeader] Generating ${ARG_OUTPUT_FILE}"
    )
    
    # Create custom target
    add_custom_target(${TARGET_NAME} ALL DEPENDS ${ARG_OUTPUT_FILE})
    set_target_properties(${TARGET_NAME} PROPERTIES FOLDER codegen)
    add_dependencies(${TARGET_NAME} ${ARG_INPUT_JSON_TARGET})
    
    # Return target name if output variable specified
    if(ARG_OUTPUT_TARGET_VAR)
        set(${ARG_OUTPUT_TARGET_VAR} ${TARGET_NAME} PARENT_SCOPE)
    endif()
endfunction()