#pragma once

#if defined(_WIN32) || defined(_WIN64)
    #if defined(UTCOMPONENT_LIB)
        #define UTCOMPONENT_EXPORT
    #elif defined(UTCOMPONENT_DLL)
        #define UTCOMPONENT_EXPORT Q_DECL_EXPORT
    #else
        #define UTCOMPONENT_EXPORT Q_DECL_IMPORT
    #endif
#else
    #define UTCOMPONENT_EXPORT 
#endif
