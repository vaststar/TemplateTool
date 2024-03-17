#ifndef CoreFrameworkExport_h__
#define CoreFrameworkExport_h__

#ifdef COREFRAMEWORK_DLL
#ifdef COREFRAMEWORK_LIB
#define COREFRAMEWORK_EXPORT _declspec(dllexport)
#else
#define COREFRAMEWORK_EXPORT _declspec(dllimport)
#endif
#else
#define COREFRAMEWORK_EXPORT 
#endif

#endif //CoreFrameworkExport_h__