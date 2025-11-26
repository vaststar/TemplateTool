
file(GLOB_RECURSE ALL_TRANSLATION_SOURCE_FILES "${CMAKE_CURRENT_SOURCE_DIR}/../*.cpp" "${CMAKE_CURRENT_SOURCE_DIR}/../*.qml")
set(GENERATED_CPP_FILES ${UCF_CODE_GENERATOR_DIR}/UIResourceStringLoader/ResourceStringMap.cpp)
list(APPEND ALL_TRANSLATION_SOURCE_FILES ${GENERATED_CPP_FILES})

set(Translation_DIR ${CMAKE_CURRENT_SOURCE_DIR}/translations)
set(Translation_LIB Translation_obj_i18n)
set(Translation_TS_FILES        
    ${Translation_DIR}/app_translations_en.ts
    ${Translation_DIR}/app_translations_zh-CN.ts
)

add_library(${Translation_LIB} OBJECT)

qt_add_translations(
    ${Translation_LIB}  
    RESOURCE_PREFIX "/i18n"
    TS_FILE_BASE app_translations
    TS_FILE_DIR ${Translation_DIR}
    TS_FILES ${Translation_TS_FILES}
    SOURCES ${ALL_TRANSLATION_SOURCE_FILES}
)

SetIDEFolder(
    TARGET_NAMES
        ${Translation_LIB}
        ${Translation_LIB}_other_files
        ${Translation_LIB}_lupdate
        ${Translation_LIB}_lrelease
        ${Translation_LIB}_${Translation_LIB}_translations
        ${Translation_LIB}_resources_3
        update_translations
        release_translations
    FOLDER_NAME
        UI/translations
)

if (TARGET ${Translation_LIB}_lupdate)
    add_dependencies(${Translation_LIB}_lupdate UIResourceStringLoader)
else()
    message(STATUS "[I18N] ${Translation_LIB}_lupdate not found, creating with qt_add_lupdate")

    qt_add_lupdate(
        LUPDATE_TARGET ${Translation_LIB}_lupdate
        TS_FILES ${Translation_TS_FILES}
        SOURCES  ${ALL_TRANSLATION_SOURCE_FILES}
        OPTIONS -no-obsolete
    )
    add_dependencies(${Translation_LIB}_lupdate UIResourceStringLoader)
endif()
add_custom_target(update_all_i18n)
add_dependencies(update_all_i18n ${Translation_LIB}_lupdate)


option(I18N_UPDATE_TS "Run lupdate during build (OFF = only when explicitly invoked)" OFF)
if(I18N_UPDATE_TS)
    message(STATUS "I18N_UPDATE_TS is ON")

    if (TARGET ${Translation_LIB}_${Translation_LIB}_translations)
    message(STATUS "got ${Translation_LIB}_${Translation_LIB}_translations")
    endif()

    if (TARGET ${Translation_LIB}_lupdate)
    message(STATUS "got ${Translation_LIB}_lupdate")
    endif()

    if (TARGET release_translations)
    message(STATUS "got release_translations")
    endif()

    if (TARGET ${Translation_LIB}_lrelease AND TARGET ${Translation_LIB}_lupdate)
        message(STATUS "add dependencies: ${Translation_LIB}_lrelease ${Translation_LIB}_lupdate")

        add_dependencies(${Translation_LIB}_lrelease ${Translation_LIB}_lupdate)
    endif()
    if (TARGET ${Translation_LIB}_${Translation_LIB}_translations AND TARGET ${Translation_LIB}_lupdate)
        message(STATUS "add dependencies: ${Translation_LIB}_${Translation_LIB}_translations ${Translation_LIB}_lupdate")
        add_dependencies(${Translation_LIB}_${Translation_LIB}_translations ${Translation_LIB}_lupdate)
    endif()
    if (TARGET release_translations AND TARGET update_translations)
        message(STATUS "add dependencies: release_translations update_translations")
        add_dependencies(release_translations update_translations)
    endif()
else()
    message(STATUS "I18N_UPDATE_TS is OFF")
endif()

if (TARGET release_translations)
    add_dependencies(${Translation_LIB} release_translations )
endif()
