#ifndef DataWarehouseExportExport_h__
#define DataWarehouseExportExport_h__

#ifdef WIN32
#pragma warning(disable:4251)//for template class member with dll-export, it will report warning, so, just disable it
#endif

#ifdef DATAWAREHOUSE_DLL
#ifdef DATAWAREHOUSE_LIB
#define DataWarehouse_EXPORT _declspec(dllexport)
#else
#define DataWarehouse_EXPORT _declspec(dllimport)
#endif
#else
#define DataWarehouse_EXPORT 
#endif

#endif //DataWarehouseExportExport_h__