include_guard()

# ==========================================
# Script mode: Generate app info/resource files
# ==========================================
if(CMAKE_SCRIPT_MODE_FILE)
    # Validate required variables
    if(NOT DEFINED INPUT_JSON_FILE)
        message(FATAL_ERROR "[GenerateAppInfoFiles] INPUT_JSON_FILE variable not defined")
    endif()
    if(NOT DEFINED INPUT_TEMPLATE_FILE)
        message(FATAL_ERROR "[GenerateAppInfoFiles] INPUT_TEMPLATE_FILE variable not defined")
    endif()
    if(NOT DEFINED OUTPUT_H)
        message(FATAL_ERROR "[GenerateAppInfoFiles] OUTPUT_H variable not defined")
    endif()
    
    # Validate input files exist
    if(NOT EXISTS "${INPUT_JSON_FILE}")
        message(FATAL_ERROR "[GenerateAppInfoFiles] Input JSON file not found: ${INPUT_JSON_FILE}")
    endif()
    if(NOT EXISTS "${INPUT_TEMPLATE_FILE}")
        message(FATAL_ERROR "[GenerateAppInfoFiles] Template file not found: ${INPUT_TEMPLATE_FILE}")
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

    # Calculate version strings (for Windows RC files)
    set(FILE_VERSION "${VERSION_MAJOR},${VERSION_MINOR},${VERSION_PATCH},${VERSION_BUILD}")
    set(PRODUCT_VERSION "${VERSION_MAJOR},${VERSION_MINOR},${VERSION_PATCH},${VERSION_BUILD}")
    set(FILE_VERSION_STR "${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}.${VERSION_BUILD}")
    set(PRODUCT_VERSION_STR "${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}.${VERSION_BUILD}")

    configure_file(${INPUT_TEMPLATE_FILE} ${OUTPUT_H} @ONLY)
    message(STATUS "[GenerateAppInfoFiles] Generated ${OUTPUT_H}")
endif()

# ==========================================
# Function: generate_app_info_files
# ==========================================
function(generate_app_info_files)
    set(oneValueArgs INPUT_JSON_FILE INPUT_JSON_TARGET INPUT_VERSION_TEMPLATE OUTPUT_FILE INTERNAL_NAME FILE_DESCRIPTION ORIGINAL_FILENAME OUTPUT_TARGET_VAR)
    cmake_parse_arguments(ARG "" "${oneValueArgs}" "" ${ARGN})

    # Validate required arguments
    if(NOT ARG_INPUT_VERSION_TEMPLATE)
        message(FATAL_ERROR "[GenerateAppInfoFiles] INPUT_VERSION_TEMPLATE is required")
    endif()
    if(NOT ARG_INPUT_JSON_TARGET)
        message(FATAL_ERROR "[GenerateAppInfoFiles] INPUT_JSON_TARGET is required")
    endif()
    if(NOT ARG_OUTPUT_FILE)
        message(FATAL_ERROR "[GenerateAppInfoFiles] OUTPUT_FILE is required")
    endif()
    if(NOT ARG_INPUT_JSON_FILE)
        message(FATAL_ERROR "[GenerateAppInfoFiles] INPUT_JSON_FILE is required")
    endif()

    # Generate unique target name based on output file path hash
    string(MD5 PATH_HASH "${ARG_OUTPUT_FILE}")
    string(SUBSTRING "${PATH_HASH}" 0 8 SHORT_HASH)
    get_filename_component(OUTPUT_NAME "${ARG_OUTPUT_FILE}" NAME_WE)
    set(TARGET_NAME "generate_${OUTPUT_NAME}_${SHORT_HASH}")

    message(STATUS "")
    message(STATUS "============================================================")
    message(STATUS "[GenerateAppInfoFiles] Configuring app info file generation")
    message(STATUS "============================================================")
    message(STATUS "  Output File  : ${ARG_OUTPUT_FILE}")
    message(STATUS "  Template     : ${ARG_INPUT_VERSION_TEMPLATE}")
    message(STATUS "  Input JSON   : ${ARG_INPUT_JSON_FILE}")
    message(STATUS "  JSON Target  : ${ARG_INPUT_JSON_TARGET}")
    message(STATUS "  Target Name  : ${TARGET_NAME}")
    if(ARG_INTERNAL_NAME)
        message(STATUS "  Internal Name: ${ARG_INTERNAL_NAME}")
    endif()
    if(ARG_FILE_DESCRIPTION)
        message(STATUS "  Description  : ${ARG_FILE_DESCRIPTION}")
    endif()
    if(ARG_ORIGINAL_FILENAME)
        message(STATUS "  Original Name: ${ARG_ORIGINAL_FILENAME}")
    endif()
    message(STATUS "------------------------------------------------------------")
   
    # Add custom command to generate resource file
    add_custom_command(
        OUTPUT ${ARG_OUTPUT_FILE}
        COMMAND ${CMAKE_COMMAND} 
            -DINPUT_JSON_FILE=${ARG_INPUT_JSON_FILE}
            -DINPUT_TEMPLATE_FILE=${ARG_INPUT_VERSION_TEMPLATE}
            -DFILE_DESCRIPTION=${ARG_FILE_DESCRIPTION}
            -DINTERNAL_NAME=${ARG_INTERNAL_NAME}
            -DORIGINAL_FILENAME=${ARG_ORIGINAL_FILENAME}
            -DOUTPUT_H=${ARG_OUTPUT_FILE}
            -P "${CMAKE_CURRENT_FUNCTION_LIST_DIR}/GenerateAppInfoFiles.cmake"
        DEPENDS ${ARG_INPUT_JSON_FILE} ${ARG_INPUT_VERSION_TEMPLATE}
        COMMENT "[GenerateAppInfoFiles] Generating ${ARG_OUTPUT_FILE}"
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
