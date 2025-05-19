#pragma once

#if defined(_WIN32) || defined(_WIN64)
    #if defined(UIComponentBase_DLL)
        #define UIComponentBase_EXPORT Q_DECL_EXPORT
    #else
        #define UIComponentBase_EXPORT Q_DECL_IMPORT
    #endif
#else
    #define UIComponentBase_EXPORT 
#endif
