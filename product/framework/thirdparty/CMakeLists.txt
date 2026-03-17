# =============================================================================
# Auto-extract large binary files (read from large_files.json)
# =============================================================================
set(SCRIPTS_DIR "${CMAKE_CURRENT_SOURCE_DIR}/scripts")
set(LARGE_FILES_JSON "${CMAKE_CURRENT_SOURCE_DIR}/large_files.json")
set(NEED_EXTRACT FALSE)

# Determine current platform
if(WIN32)
    set(TARGET_PLATFORM "windows")
elseif(ANDROID)
    set(TARGET_PLATFORM "android")
elseif(APPLE)
    if(CMAKE_SYSTEM_NAME STREQUAL "iOS")
        set(TARGET_PLATFORM "ios")
    else()
        set(TARGET_PLATFORM "macos")
    endif()
else()
    set(TARGET_PLATFORM "linux")
endif()

# Read JSON and check files
if(EXISTS "${LARGE_FILES_JSON}")
    file(READ "${LARGE_FILES_JSON}" JSON_CONTENT)
    string(JSON FILE_COUNT LENGTH "${JSON_CONTENT}" "files")

    if(FILE_COUNT GREATER 0)
        math(EXPR LAST_INDEX "${FILE_COUNT} - 1")
        foreach(INDEX RANGE 0 ${LAST_INDEX})
            string(JSON FILE_PATH GET "${JSON_CONTENT}" "files" ${INDEX} "path")
            string(JSON FILE_PLATFORM GET "${JSON_CONTENT}" "files" ${INDEX} "platform")

            if(FILE_PLATFORM STREQUAL TARGET_PLATFORM)
                set(FULL_PATH "${CMAKE_CURRENT_SOURCE_DIR}/${FILE_PATH}")
                set(ZIP_PATH "${FULL_PATH}.zip")
                if(NOT EXISTS "${FULL_PATH}" AND EXISTS "${ZIP_PATH}")
                    set(NEED_EXTRACT TRUE)
                    break()
                endif()
            endif()
        endforeach()
    endif()
endif()

# Extract if missing files found
if(NEED_EXTRACT)
    message(STATUS "[thirdparty] Extracting large files for ${TARGET_PLATFORM}...")
    if(WIN32)
        execute_process(
            COMMAND powershell -ExecutionPolicy Bypass -File "${SCRIPTS_DIR}/extract_large_files.ps1" -Platform ${TARGET_PLATFORM}
            WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
        )
    else()
        execute_process(
            COMMAND bash "${SCRIPTS_DIR}/extract_large_files.sh" ${TARGET_PLATFORM}
            WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
        )
    endif()
endif()

# =============================================================================
# thirdparty lib
# =============================================================================
add_subdirectory(compression)
add_subdirectory(archive)
add_subdirectory(openssl)
add_subdirectory(json)
add_subdirectory(Catch2)
add_subdirectory(sqlite)
add_subdirectory(curl)
add_subdirectory(opencv)
add_subdirectory(magic_enum)
add_subdirectory(trompeloeil)
add_subdirectory(qrcodegen)
add_subdirectory(ffmpeg)

#my lib
add_subdirectory(threadpool)
add_subdirectory(logger)
