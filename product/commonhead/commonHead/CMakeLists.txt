if (WIN32)
    set(ADD_COMMONHEAD_DEFINITIONS COMMONHEAD_DLL) #COMMONHEAD_DLL COMMONHEAD_LIB
else()
    set(ADD_COMMONHEAD_DEFINITIONS)
endif()

add_subdirectory(common_file)
add_subdirectory(commonHeadUtils)
add_subdirectory(commonHeadManager)
add_subdirectory(commonHeadFramework)
add_subdirectory(viewModels)