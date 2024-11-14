#pragma once

#ifdef UIManager_DLL
#define UIManager_EXPORT Q_DECL_EXPORT
#else
#define UIManager_EXPORT Q_DECL_IMPORT
#endif
