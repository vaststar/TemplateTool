#pragma once

#ifdef UICore_DLL
#define UICore_EXPORT Q_DECL_EXPORT
#else
#define UICore_EXPORT Q_DECL_IMPORT
#endif

// #if defined(MYSHAREDLIB_LIBRARY)
// #  define MYSHAREDLIB_EXPORT Q_DECL_EXPORT
// #else
// #  define MYSHAREDLIB_EXPORT Q_DECL_IMPORT
// #endif
