if (WIN32)
    set(ADD_UIUtilities_DEFINITIONS UIUtilities_DLL) #UIUtilities_DLL UIUtilities_LIB
else()
    set(ADD_UIUtilities_DEFINITIONS)
endif()

add_subdirectory(common_file)
add_subdirectory(UIPlatformUtils)