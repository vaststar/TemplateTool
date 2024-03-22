#pragma once

#ifdef Utilities_DLL
#ifdef Utilities_LIB
#define Notification_LIB_API _declspec(dllexport)
#else
#define Notification_LIB_API _declspec(dllimport)
#endif
#else 
#define Notification_LIB_API 
#endif