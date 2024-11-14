#pragma once

#ifdef UIFabrication_DLL
#define UIFabrication_EXPORT Q_DECL_EXPORT
#else
#define UIFabrication_EXPORT Q_DECL_IMPORT
#endif
