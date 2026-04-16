#include "OSUtils_Win.h"
#include <windows.h>
#include <format>
#include <intrin.h>
#include <array>
#include <dxgi.h>
#pragma comment(lib, "dxgi.lib")


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

std::string OSUtils_Win::getCPUInfo()
{
    std::array<int, 4> cpuInfo;
    char brand[0x40] = { 0 };

    // 0x80000002, 0x80000003, 0x80000004 这三个调用返回 CPU 品牌字符串的不同部分
    for (int i = 0; i < 3; i++) {
        __cpuid(cpuInfo.data(), 0x80000002 + i);
        memcpy(brand + i * 16, cpuInfo.data(), sizeof(cpuInfo));
    }

    return std::string(brand);
}

std::string OSUtils_Win::getSystemLanguage()
{
    wchar_t localeName[LOCALE_NAME_MAX_LENGTH] = {0};
    if (GetUserDefaultLocaleName(localeName, LOCALE_NAME_MAX_LENGTH)) {
        char locale[LOCALE_NAME_MAX_LENGTH] = {0};
        WideCharToMultiByte(CP_UTF8, 0, localeName, -1, locale, LOCALE_NAME_MAX_LENGTH, nullptr, nullptr);
        return std::string(locale);
    }
    return "en-US";
}

MemoryInfo OSUtils_Win::getMemoryInfo()
{
    MEMORYSTATUSEX statex;
    statex.dwLength = sizeof(statex);
    if (GlobalMemoryStatusEx(&statex))
    {
        return MemoryInfo{ statex.ullTotalPhys, statex.ullAvailPhys };
    }
    else
    {
        return MemoryInfo{ 0, 0 }; // Return zeroed memory info on failure
    }
}

std::string OSUtils_Win::getGPUInfo()
{
    IDXGIFactory* factory = nullptr;
    HRESULT hr = CreateDXGIFactory(__uuidof(IDXGIFactory), (void**)&factory);
    if (FAILED(hr) || !factory) return "";

    std::string result;
    IDXGIAdapter* adapter = nullptr;
    for (UINT i = 0; factory->EnumAdapters(i, &adapter) != DXGI_ERROR_NOT_FOUND; ++i) {
        DXGI_ADAPTER_DESC desc;
        adapter->GetDesc(&desc);
        // wchar_t 转 std::string（UTF-8简易版）
        char name[128];
        wcstombs(name, desc.Description, sizeof(name));
        result += "GPU: " + std::string(name) + "\n";
        adapter->Release();
    }
    factory->Release();
    return result;
}
} // namespace ucf::utilities