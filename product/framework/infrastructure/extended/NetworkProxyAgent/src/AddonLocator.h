#pragma once

#include <string>

namespace ucf::agents::detail {

/// Locates the proxy addon executable and related files relative to the
/// NetworkProxyAgent shared library (DLL/dylib/so) at runtime.
///
/// On Windows: Uses GetModuleHandleEx/GetModuleFileName to find the DLL directory.
/// On Unix:    Uses dladdr to find the shared library directory.
///
/// Expected layout at runtime:
///   <lib_dir>/NetworkProxyAgent.dll  (or .dylib / .so)
///   <lib_dir>/../Resources/proxy_addon/proxy_addon   (macOS .app bundle)
///   <lib_dir>/proxy_addon/proxy_addon.exe      (Windows/Linux build + install)
///   <lib_dir>/../proxy_addon/proxy_addon.exe   (alternate sibling layout)
///   ~/.mitmproxy/mitmproxy-ca-cert.cer  (Windows) / .pem (Unix)
class AddonLocator
{
public:
    /// Find the full path to the proxy_addon executable.
    /// Returns empty string if not found.
    static std::string findAddonExecutable();

    /// Find the mitmproxy CA certificate path.
    /// Returns the default mitmproxy cert path based on the user's home directory.
    static std::string findCACertPath();

private:
    /// Get the directory containing the NetworkProxyAgent shared library.
    static std::string getLibraryDirectory();
};

} // namespace ucf::agents::detail
