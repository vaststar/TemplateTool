#include "AddonLocator.h"
#include "NetworkProxyAgentLogger.h"

#include <cstdlib>

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#else
#include <dlfcn.h>
#include <unistd.h>
#include <climits>
#endif

namespace ucf::agents::detail {

// ═══════════════════════════════════════════════════════════════
//  getLibraryDirectory — find the directory containing this DLL
// ═══════════════════════════════════════════════════════════════

#ifdef _WIN32

/// Dummy function whose address lives inside our DLL; used by GetModuleHandleEx.
static void dummyForModuleHandle() {}

std::string AddonLocator::getLibraryDirectory()
{
    HMODULE hModule = nullptr;
    if (!GetModuleHandleExA(
            GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
            reinterpret_cast<LPCSTR>(&dummyForModuleHandle),
            &hModule))
    {
        NPA_LOG_ERROR("GetModuleHandleExA failed, error=" << GetLastError());
        return {};
    }

    char path[MAX_PATH] = {};
    DWORD len = GetModuleFileNameA(hModule, path, MAX_PATH);
    if (len == 0 || len >= MAX_PATH)
    {
        NPA_LOG_ERROR("GetModuleFileNameA failed, error=" << GetLastError());
        return {};
    }

    std::string fullPath(path, len);
    auto lastSep = fullPath.find_last_of("\\/");
    if (lastSep == std::string::npos)
    {
        return ".";
    }
    return fullPath.substr(0, lastSep);
}

#else // Unix (macOS / Linux)

std::string AddonLocator::getLibraryDirectory()
{
    Dl_info info{};
    // Use the address of this very function to locate the shared library
    if (dladdr(reinterpret_cast<void*>(&AddonLocator::getLibraryDirectory), &info) == 0
        || info.dli_fname == nullptr)
    {
        NPA_LOG_ERROR("dladdr failed");
        return {};
    }

    std::string fullPath = info.dli_fname;

    // Resolve to absolute path if relative
    if (!fullPath.empty() && fullPath[0] != '/')
    {
        char resolved[PATH_MAX] = {};
        if (realpath(fullPath.c_str(), resolved))
        {
            fullPath = resolved;
        }
    }

    auto lastSep = fullPath.rfind('/');
    if (lastSep == std::string::npos)
    {
        return ".";
    }
    return fullPath.substr(0, lastSep);
}

#endif

// ═══════════════════════════════════════════════════════════════
//  findAddonExecutable
// ═══════════════════════════════════════════════════════════════

std::string AddonLocator::findAddonExecutable()
{
    std::string libDir = getLibraryDirectory();
    if (libDir.empty())
    {
        NPA_LOG_WARN("Cannot determine library directory, addon search will fail");
        return {};
    }

    NPA_LOG_DEBUG("Library directory: " << libDir);

    // Search paths relative to the lib directory
    // Typical layout: <build>/bin/Release/ or <build>/bin/Debug/
    //   addon at:     <build>/bin/Release/proxy_addon/proxy_addon.exe (Win)
    //   or:           <lib_dir>/proxy_addon/proxy_addon  (Unix)
#ifdef _WIN32
    static const char* kExeName = "proxy_addon.exe";
#else
    static const char* kExeName = "proxy_addon";
#endif

    // Try: <lib_dir>/proxy_addon/<exe>
    {
        std::string candidate = libDir + "/proxy_addon/" + kExeName;
        NPA_LOG_DEBUG("Trying addon path: " << candidate);
#ifdef _WIN32
        DWORD attrs = GetFileAttributesA(candidate.c_str());
        if (attrs != INVALID_FILE_ATTRIBUTES && !(attrs & FILE_ATTRIBUTE_DIRECTORY))
        {
            NPA_LOG_INFO("Found addon at: " << candidate);
            return candidate;
        }
#else
        if (access(candidate.c_str(), X_OK) == 0)
        {
            NPA_LOG_INFO("Found addon at: " << candidate);
            return candidate;
        }
#endif
    }

#ifdef __APPLE__
    // Try: <lib_dir>/../Resources/proxy_addon/<exe>
    // Packaged macOS app layout:
    //   mainEntry.app/Contents/Frameworks/NetworkProxyAgent.dylib
    //   mainEntry.app/Contents/Resources/proxy_addon/proxy_addon
    {
        std::string candidate = libDir + "/../Resources/proxy_addon/" + kExeName;
        NPA_LOG_DEBUG("Trying addon path: " << candidate);
        if (access(candidate.c_str(), X_OK) == 0)
        {
            NPA_LOG_INFO("Found addon at: " << candidate);
            return candidate;
        }
    }
#endif

    // Try: <lib_dir>/../proxy_addon/<exe>
    {
        std::string candidate = libDir + "/../proxy_addon/" + kExeName;
        NPA_LOG_DEBUG("Trying addon path: " << candidate);
#ifdef _WIN32
        DWORD attrs = GetFileAttributesA(candidate.c_str());
        if (attrs != INVALID_FILE_ATTRIBUTES && !(attrs & FILE_ATTRIBUTE_DIRECTORY))
        {
            NPA_LOG_INFO("Found addon at: " << candidate);
            return candidate;
        }
#else
        if (access(candidate.c_str(), X_OK) == 0)
        {
            NPA_LOG_INFO("Found addon at: " << candidate);
            return candidate;
        }
#endif
    }

    NPA_LOG_WARN("proxy_addon executable not found relative to: " << libDir);
    return {};
}

// ═══════════════════════════════════════════════════════════════
//  findCACertPath
// ═══════════════════════════════════════════════════════════════

std::string AddonLocator::findCACertPath()
{
#ifdef _WIN32
    const char* home = std::getenv("USERPROFILE");
    if (!home)
    {
        return {};
    }
    return std::string(home) + "\\.mitmproxy\\mitmproxy-ca-cert.cer";
#else
    const char* home = std::getenv("HOME");
    if (!home)
    {
        return {};
    }
    return std::string(home) + "/.mitmproxy/mitmproxy-ca-cert.pem";
#endif
}

} // namespace ucf::agents::detail
