include_guard()
include (LinkTargetIncludeDirectories)

function(BuildUnitTestModule)
        message(STATUS "====Start Build Test Module====")
        set(options)
        set(oneValueArg MODULE_NAME IDE_FOLDER)
        set(multiValueArgs
            TARGET_SOURCE TARGET_DEPENDENICES TARGET_INCLUDE_FOLDER
        )
        cmake_parse_arguments(MODULE "${options}" "${oneValueArg}" "${multiValueArgs}" ${ARGN})

        message(STATUS "Parse Args Results:")
        message(STATUS "MODULE_NAME: ${MODULE_MODULE_NAME}")
        message(STATUS "TARGET_SOURCE: ${MODULE_TARGET_SOURCE}")
        message(STATUS "TARGET_DEPENDENICES: ${MODULE_TARGET_DEPENDENICES}")
        message(STATUS "TARGET_INCLUDE_FOLDER: ${MODULE_TARGET_INCLUDE_FOLDER}")
        message(STATUS "IDE_FOLDER: ${MODULE_IDE_FOLDER}")
        
        message(STATUS "***create test exe: ${MODULE_MODULE_NAME}***")

        ##build exe
        add_executable(${MODULE_MODULE_NAME} "")
        
        target_sources(${MODULE_MODULE_NAME}
            PRIVATE ${MODULE_TARGET_SOURCE}
        )

        target_compile_features(${MODULE_MODULE_NAME} PRIVATE cxx_std_20)
        set_target_properties(${MODULE_MODULE_NAME} PROPERTIES LINKER_LANGUAGE CXX)
        set_target_properties(${MODULE_MODULE_NAME} PROPERTIES CXX_EXTENSIONS OFF)
        if(DEFINED  MODULE_IDE_FOLDER)
            set_target_properties(${MODULE_MODULE_NAME} PROPERTIES FOLDER ${MODULE_IDE_FOLDER})
        endif()
        
        LinkTargetIncludeDirectories(
            MODULE_NAME ${MODULE_MODULE_NAME}
            TARGET_INCLUDE_DIRECTORIES_PRIVATE ${MODULE_TARGET_INCLUDE_FOLDER}
        )
        
        if(DEFINED MODULE_TARGET_DEPENDENICES)
            message(STATUS "will add private link to ${MODULE_MODULE_NAME}, link librarys: ${MODULE_TARGET_DEPENDENICES}")
            target_link_libraries(${MODULE_MODULE_NAME}  PRIVATE ${MODULE_TARGET_DEPENDENICES})
        endif()

        #for project tree view
        source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${MODULE_TARGET_SOURCE} ${MODULE_TARGET_SOURCE_PUBLIC_HEADER})

        message(STATUS "====Finish Build Module: ${MODULE_MODULE_NAME}====")
endfunction()