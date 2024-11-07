#include "OSUtils_Win.h"
#include <windows.h>
#include <format>

#define DECLARE_DLL_FUNCTION(fn, type, dll) \
    auto fn = reinterpret_cast<type>(GetProcAddress(GetModuleHandleW(L##dll), #fn))

namespace ucf::utilities{
std::string OSUtils_Win::getOSVersion()
{
    constexpr NTSTATUS kStatusSuccess = 0L;
	DECLARE_DLL_FUNCTION(RtlGetVersion, NTSTATUS(WINAPI*)(PRTL_OSVERSIONINFOW), "ntdll.dll");
	if (!RtlGetVersion) 
    {
		return "";
	}

	RTL_OSVERSIONINFOW ovi{ sizeof(ovi) };
	if (RtlGetVersion(&ovi) == kStatusSuccess) 
    {
        return std::format("{}.{} (Build {})", std::to_string(ovi.dwMajorVersion), std::to_string(ovi.dwMinorVersion), std::to_string(ovi.dwBuildNumber));;
	}
    else 
    {
        return "Unknown Windows version";
    }
}
}