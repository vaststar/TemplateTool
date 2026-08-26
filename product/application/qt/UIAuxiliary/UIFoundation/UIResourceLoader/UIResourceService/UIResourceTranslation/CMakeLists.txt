include(GetRequiredTargetProperty)

get_required_target_property(
    TARGET UIResourceStringLoader
    PROPERTY UCF_GENERATED_TRANSLATION_SOURCE
    OUTPUT_VARIABLE GENERATED_RESOURCE_STRING_SOURCE
)

if(QT_TRANSLATION_CODE_DIR)
    set(TRANSLATION_CODE_DIR ${QT_TRANSLATION_CODE_DIR})
    message(STATUS "[I18N] Using external code dir: ${TRANSLATION_CODE_DIR}")
else()
    set(TRANSLATION_CODE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../../../..")
    message(STATUS "[I18N] Using default code dir: ${TRANSLATION_CODE_DIR}")
endif()

file(GLOB_RECURSE ALL_TRANSLATION_SOURCE_FILES
    CONFIGURE_DEPENDS
    "${TRANSLATION_CODE_DIR}/*.cpp"
    "${TRANSLATION_CODE_DIR}/*.qml"
)

# Exclude the developer-only gallery (UIToolKitGallery) from translation
# extraction: its strings are demo content and should not ship in the .ts files.
list(FILTER ALL_TRANSLATION_SOURCE_FILES EXCLUDE REGEX "/UIToolKitGallery/")

list(APPEND ALL_TRANSLATION_SOURCE_FILES
    "${GENERATED_RESOURCE_STRING_SOURCE}")

if(QT_TRANSLATION_DIR)
    set(TRANSLATION_DIR ${QT_TRANSLATION_DIR})
    message(STATUS "[I18N] Using external translation dir: ${TRANSLATION_DIR}")
else()
    set(TRANSLATION_DIR ${CMAKE_CURRENT_SOURCE_DIR}/translations)
    message(STATUS "[I18N] Using default translation dir: ${TRANSLATION_DIR}")
endif()
set(TRANSLATION_TARGET UIResourceTranslation)
set(TRANSLATION_LOCALES
    en
    zh-Hans
    zh-Hant
)

list(LENGTH TRANSLATION_LOCALES TRANSLATION_LOCALE_COUNT)
set(TRANSLATION_LOCALE_INITIALIZERS "")
foreach(TRANSLATION_LOCALE IN LISTS TRANSLATION_LOCALES)
    string(APPEND TRANSLATION_LOCALE_INITIALIZERS
        "    std::string_view{\"${TRANSLATION_LOCALE}\"},\n"
    )
endforeach()

set(TRANSLATION_CATALOG_HEADER_BASE_DIR
    ${UCF_CODE_GENERATOR_DIR}
)
set(TRANSLATION_CATALOG_OUTPUT_DIR
    ${TRANSLATION_CATALOG_HEADER_BASE_DIR}/UIResourceTranslation
)
set(TRANSLATION_CATALOG_OUTPUT_PATH
    ${TRANSLATION_CATALOG_OUTPUT_DIR}/GeneratedTranslationCatalog.h
)

file(MAKE_DIRECTORY ${TRANSLATION_CATALOG_OUTPUT_DIR})
configure_file(
    templates/GeneratedTranslationCatalog.h.in
    ${TRANSLATION_CATALOG_OUTPUT_PATH}
    @ONLY
)

include(BuildInterface)
BuildInterface(
    NO_INSTALL
    MODULE_NAME UIResourceTranslationCatalog
    TARGET_SOURCE_HEADER_BASE_DIR
        ${TRANSLATION_CATALOG_HEADER_BASE_DIR}
    TARGET_SOURCE_PUBLIC_HEADER
        ${TRANSLATION_CATALOG_OUTPUT_PATH}
    TARGET_INCLUDE_DIRECTORIES_BUILD_INTERFACE
        ${TRANSLATION_CATALOG_HEADER_BASE_DIR}
    IDE_FOLDER
        UI/codegen
)

set(TRANSLATION_TS_FILES)
foreach(TRANSLATION_LOCALE IN LISTS TRANSLATION_LOCALES)
    list(APPEND TRANSLATION_TS_FILES
        "${TRANSLATION_DIR}/app_translations_${TRANSLATION_LOCALE}.ts"
    )
endforeach()

add_library(${TRANSLATION_TARGET} OBJECT)

qt_add_translations(
    ${TRANSLATION_TARGET}
    RESOURCE_PREFIX "/i18n"
    TS_FILE_BASE app_translations
    TS_FILE_DIR ${TRANSLATION_DIR}
    TS_FILES ${TRANSLATION_TS_FILES}
    SOURCES ${ALL_TRANSLATION_SOURCE_FILES}
    LUPDATE_OPTIONS -no-obsolete
)


if (TARGET ${TRANSLATION_TARGET}_lupdate)
    add_dependencies(${TRANSLATION_TARGET}_lupdate UIResourceStringLoader)
else()
    message(STATUS "[I18N] ${TRANSLATION_TARGET}_lupdate not found, creating with qt_add_lupdate")

    qt_add_lupdate(
        LUPDATE_TARGET ${TRANSLATION_TARGET}_lupdate
        TS_FILES ${TRANSLATION_TS_FILES}
        SOURCES  ${ALL_TRANSLATION_SOURCE_FILES}
        OPTIONS -no-obsolete
    )
    add_dependencies(${TRANSLATION_TARGET}_lupdate UIResourceStringLoader)
endif()

# Normalize codegen paths in .ts files after lupdate
# This prevents git diff when switching between macOS and Windows builds
set(NORMALIZE_TS_SCRIPT "${CMAKE_CURRENT_SOURCE_DIR}/normalize_ts_paths.py")
find_package(Python3 QUIET COMPONENTS Interpreter)
if(EXISTS ${NORMALIZE_TS_SCRIPT} AND TARGET ${TRANSLATION_TARGET}_lupdate AND Python3_FOUND)
    add_custom_command(
        TARGET ${TRANSLATION_TARGET}_lupdate
        POST_BUILD
        COMMAND ${Python3_EXECUTABLE} ${NORMALIZE_TS_SCRIPT} ${TRANSLATION_TS_FILES}
        COMMENT "[I18N] Normalizing codegen paths in .ts files..."
        VERBATIM
    )
    message(STATUS "[I18N] Will normalize .ts paths after lupdate (Python3: ${Python3_EXECUTABLE})")
else()
    if(NOT Python3_FOUND)
        message(WARNING "[I18N] Python3 not found, skipping .ts path normalization")
    endif()
endif()

if (NOT TARGET ${TRANSLATION_TARGET}_lupdate)
    message(WARNING "[I18N] ${TRANSLATION_TARGET}_lupdate target could not be created.")
endif()


set(UPDATE_TRANSLATIONS_TARGET UPDATE_Translation_ALL_LIB)

add_custom_target(${UPDATE_TRANSLATIONS_TARGET})
add_dependencies(${UPDATE_TRANSLATIONS_TARGET} ${TRANSLATION_TARGET}_lupdate)

if (TARGET ${TRANSLATION_TARGET}_lrelease)
    add_dependencies(${TRANSLATION_TARGET}_lrelease ${TRANSLATION_TARGET}_lupdate)
    add_dependencies(${UPDATE_TRANSLATIONS_TARGET} ${TRANSLATION_TARGET}_lrelease)
endif()

if (TARGET ${TRANSLATION_TARGET}_${TRANSLATION_TARGET}_translations)
    add_dependencies(${TRANSLATION_TARGET}_${TRANSLATION_TARGET}_translations ${TRANSLATION_TARGET}_lupdate)
    add_dependencies(${UPDATE_TRANSLATIONS_TARGET} ${TRANSLATION_TARGET}_${TRANSLATION_TARGET}_translations)
endif()

if (TARGET release_translations AND TARGET update_translations)
    add_dependencies(release_translations update_translations)
    add_dependencies(${UPDATE_TRANSLATIONS_TARGET} release_translations)
endif()
add_dependencies(${TRANSLATION_TARGET} ${UPDATE_TRANSLATIONS_TARGET} )

if (TARGET release_translations)
    add_dependencies(${TRANSLATION_TARGET} release_translations )
endif()

SetIDEFolder(
    TARGET_NAMES
        ${TRANSLATION_TARGET}
        ${TRANSLATION_TARGET}_other_files
        ${TRANSLATION_TARGET}_lupdate
        ${TRANSLATION_TARGET}_lrelease
        ${TRANSLATION_TARGET}_${TRANSLATION_TARGET}_translations
        ${TRANSLATION_TARGET}_resources_3
        update_translations
        release_translations
        ${UPDATE_TRANSLATIONS_TARGET}
    FOLDER_NAME
        UI/UIAuxiliary/UIFoundation/translations
)
