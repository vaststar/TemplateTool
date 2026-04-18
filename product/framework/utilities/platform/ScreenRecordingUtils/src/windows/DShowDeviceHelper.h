#pragma once

#ifdef _WIN32

#include <string>
#include <vector>

namespace ucf::utilities::screenrecording {

struct DShowDeviceEntry
{
    std::string friendlyName;   // Human-readable (may contain non-ASCII)
    std::string monikerName;    // @device:cm:{...}\... — unique COM moniker
};

/// Escape special characters for FFmpeg dshow device names.
std::string escapeDshowDeviceName(const std::string& name);

/// Enumerate all DirectShow audio capture devices.
std::vector<DShowDeviceEntry> enumerateDShowCaptureDevices();

/// Returns true for DShow-compatible identifiers (monikers or friendly names).
/// Returns false for WASAPI endpoint IDs ({0.0.1.00000000}.{GUID}).
bool isDShowCompatible(const std::string& id);

/// Resolve WASAPI endpoint ID to friendly name via IMMDevice.
std::string getWasapiFriendlyName(const std::string& endpointId);

/// Validate device ID for FFmpeg dshow. WASAPI endpoint IDs are resolved
/// to matching DShow monikers; returns empty string if no match found.
std::string validateDShowDeviceId(const std::string& deviceId, const char* label);

/// Resolve a moniker name to its friendly name via DShow enumeration.
std::string monikerToFriendlyName(const std::string& moniker);

/// Format a device ID as an FFmpeg dshow audio input name.
/// Always returns the escaped friendly name, because FFmpeg dshow
/// matches by friendly name reliably across all versions.
std::string formatDShowAudioInput(const std::string& deviceId, const char* label);

} // namespace ucf::utilities::screenrecording

#endif // _WIN32
