#include "ScreenRecorder_Win.h"

#ifdef _WIN32

#include "LoggerDefine.h"

#include <cstdlib>
#include <filesystem>
#include <string>
#include <vector>

#include <ucf/Utilities/ProcessBridgeUtils/IProcessBridge.h>

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include <initguid.h>
#include <mmdeviceapi.h>
#include <functiondiscoverykeys_devpkey.h>
#include <dshow.h>

namespace ucf::utilities::screenrecording {

// ============================================================================
// Helpers (file-local)
// ============================================================================

/// Search the PATH environment variable for an executable by name.
static std::string findInPath(const std::string& name)
{
    const char* pathEnv = std::getenv("PATH");
    if (!pathEnv)
    {
        return {};
    }

    std::string pathStr(pathEnv);
    std::string::size_type start = 0;

    while (start < pathStr.size())
    {
        auto end = pathStr.find(';', start);
        if (end == std::string::npos)
        {
            end = pathStr.size();
        }

        std::string dir = pathStr.substr(start, end - start);
        if (!dir.empty())
        {
            std::filesystem::path candidate = std::filesystem::path(dir) / name;
            std::error_code ec;
            if (std::filesystem::is_regular_file(candidate, ec))
            {
                auto canonical = std::filesystem::canonical(candidate, ec);
                if (!ec)
                {
                    return canonical.string();
                }
            }
        }
        start = end + 1;
    }
    return {};
}

// ============================================================================
// Library Self-Location
// ============================================================================

static void dummyForModuleHandle_SRU() {}

std::string ScreenRecorder_Win::getLibraryDirectory()
{
    HMODULE hModule = nullptr;
    if (!GetModuleHandleExA(
            GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
            reinterpret_cast<LPCSTR>(&dummyForModuleHandle_SRU),
            &hModule))
    {
        SRU_LOG_ERROR("GetModuleHandleExA failed, error=" << GetLastError());
        return {};
    }

    char path[MAX_PATH] = {};
    DWORD len = GetModuleFileNameA(hModule, path, MAX_PATH);
    if (len == 0 || len >= MAX_PATH)
    {
        SRU_LOG_ERROR("GetModuleFileNameA failed, error=" << GetLastError());
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

// ============================================================================
// FFmpeg discovery
// ============================================================================

std::string ScreenRecorder_Win::findFFmpegPath()
{
    std::string libDir = getLibraryDirectory();
    if (libDir.empty())
    {
        SRU_LOG_WARN("Cannot determine library directory for FFmpeg auto-discovery");
        return findInPath("ffmpeg.exe");
    }

    // Typical layouts:
    //   Build:  <build>/bin/ScreenRecordingUtils.dll  →  ffmpeg.exe at <build>/bin/
    std::vector<std::string> candidates = {
        libDir + "/ffmpeg.exe",           // same dir
        libDir + "/../bin/ffmpeg.exe",    // sibling bin/
        libDir + "/ffmpeg/ffmpeg.exe"     // ffmpeg subfolder
    };

    for (const auto& candidate : candidates)
    {
        std::error_code ec;
        auto canonical = std::filesystem::canonical(candidate, ec);
        if (!ec && std::filesystem::is_regular_file(canonical, ec))
        {
            SRU_LOG_INFO("FFmpeg auto-discovered at: " << canonical.string());
            return canonical.string();
        }
    }

    std::string pathResult = findInPath("ffmpeg.exe");
    if (!pathResult.empty())
    {
        return pathResult;
    }

    SRU_LOG_WARN("FFmpeg not found via auto-discovery");
    return {};
}

std::string ScreenRecorder_Win::findFFmpegPath(const std::string& appDir)
{
    std::vector<std::string> candidates = {
        appDir + "/ffmpeg.exe",
        appDir + "/ffmpeg/ffmpeg.exe"
    };

    for (const auto& candidate : candidates)
    {
        std::error_code ec;
        auto canonical = std::filesystem::canonical(candidate, ec);
        if (!ec && std::filesystem::is_regular_file(canonical, ec))
        {
            return canonical.string();
        }
    }

    std::string pathResult = findInPath("ffmpeg.exe");
    if (!pathResult.empty())
    {
        return pathResult;
    }

    SRU_LOG_WARN("FFmpeg not found in candidates or PATH");
    return {};
}

// ============================================================================
// Audio Device Enumeration
// ============================================================================

/// Enumerate DirectShow audio capture device friendly names.
/// These names match what FFmpeg's dshow filter expects.
static std::vector<std::string> enumerateDShowCaptureDeviceNames()
{
    std::vector<std::string> names;

    ICreateDevEnum* pDevEnum = nullptr;
    HRESULT hr = CoCreateInstance(CLSID_SystemDeviceEnum, nullptr, CLSCTX_INPROC_SERVER,
                                  IID_ICreateDevEnum, reinterpret_cast<void**>(&pDevEnum));
    if (FAILED(hr) || !pDevEnum)
    {
        return names;
    }

    IEnumMoniker* pEnum = nullptr;
    hr = pDevEnum->CreateClassEnumerator(CLSID_AudioInputDeviceCategory, &pEnum, 0);
    if (hr == S_OK && pEnum)
    {
        IMoniker* pMoniker = nullptr;
        while (pEnum->Next(1, &pMoniker, nullptr) == S_OK)
        {
            IPropertyBag* pPropBag = nullptr;
            if (SUCCEEDED(pMoniker->BindToStorage(nullptr, nullptr, IID_IPropertyBag,
                                                   reinterpret_cast<void**>(&pPropBag))))
            {
                VARIANT var;
                VariantInit(&var);
                if (SUCCEEDED(pPropBag->Read(L"FriendlyName", &var, nullptr)))
                {
                    if (var.vt == VT_BSTR && var.bstrVal)
                    {
                        int len = WideCharToMultiByte(CP_UTF8, 0, var.bstrVal, -1,
                                                       nullptr, 0, nullptr, nullptr);
                        std::string name(len - 1, '\0');
                        WideCharToMultiByte(CP_UTF8, 0, var.bstrVal, -1,
                                            name.data(), len, nullptr, nullptr);
                        names.push_back(std::move(name));
                    }
                }
                VariantClear(&var);
                pPropBag->Release();
            }
            pMoniker->Release();
        }
        pEnum->Release();
    }

    pDevEnum->Release();
    return names;
}

std::vector<AudioDeviceInfo> ScreenRecorder_Win::enumerateAudioDevices()
{
    std::vector<AudioDeviceInfo> devices;

    HRESULT hr = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
    bool needUninit = SUCCEEDED(hr);

    IMMDeviceEnumerator* pEnumerator = nullptr;
    hr = CoCreateInstance(__uuidof(MMDeviceEnumerator), nullptr,
                          CLSCTX_ALL, __uuidof(IMMDeviceEnumerator),
                          reinterpret_cast<void**>(&pEnumerator));
    if (FAILED(hr) || !pEnumerator)
    {
        if (needUninit) CoUninitialize();
        return devices;
    }

    // Helper lambda to convert wide device name to UTF-8
    auto getDeviceName = [](IPropertyStore* pProps) -> std::string {
        PROPVARIANT varName;
        PropVariantInit(&varName);
        pProps->GetValue(PKEY_Device_FriendlyName, &varName);
        std::string name;
        if (varName.vt == VT_LPWSTR)
        {
            int len = WideCharToMultiByte(CP_UTF8, 0, varName.pwszVal, -1,
                                           nullptr, 0, nullptr, nullptr);
            name.resize(len - 1);
            WideCharToMultiByte(CP_UTF8, 0, varName.pwszVal, -1,
                                name.data(), len, nullptr, nullptr);
        }
        PropVariantClear(&varName);
        return name;
    };

    // Helper lambda to get WASAPI endpoint ID as UTF-8
    auto getEndpointId = [](IMMDevice* pDevice) -> std::string {
        LPWSTR pwszId = nullptr;
        pDevice->GetId(&pwszId);
        std::string id;
        if (pwszId)
        {
            int len = WideCharToMultiByte(CP_UTF8, 0, pwszId, -1,
                                           nullptr, 0, nullptr, nullptr);
            id.resize(len - 1);
            WideCharToMultiByte(CP_UTF8, 0, pwszId, -1,
                                id.data(), len, nullptr, nullptr);
            CoTaskMemFree(pwszId);
        }
        return id;
    };

    // ── 1a) Microphone devices directly from DirectShow ──
    // FFmpeg uses -f dshow, so we MUST use DirectShow names.
    // WASAPI and DirectShow can report completely different friendly names
    // for the same hardware, so matching between them is unreliable.
    auto dshowNames = enumerateDShowCaptureDeviceNames();
    for (const auto& dshowName : dshowNames)
    {
        AudioDeviceInfo info;
        info.displayName = dshowName;
        info.id = dshowName;
        info.isInput = true;
        info.deviceType = AudioDeviceType::Microphone;
        devices.push_back(std::move(info));
        SRU_LOG_DEBUG("DShow mic device: " << dshowName);
    }

    // ── 1b) eCapture: Non-mic capture devices only (Stereo Mix, loopback) ──
    // These are used with dshow as well, but are enumerated via WASAPI
    // because DirectShow lumps them together with mics.
    IMMDeviceCollection* pCaptureCollection = nullptr;
    hr = pEnumerator->EnumAudioEndpoints(eCapture, DEVICE_STATE_ACTIVE, &pCaptureCollection);
    if (SUCCEEDED(hr) && pCaptureCollection)
    {
        UINT count = 0;
        pCaptureCollection->GetCount(&count);
        for (UINT i = 0; i < count; i++)
        {
            IMMDevice* pDevice = nullptr;
            if (SUCCEEDED(pCaptureCollection->Item(i, &pDevice)) && pDevice)
            {
                IPropertyStore* pProps = nullptr;
                pDevice->OpenPropertyStore(STGM_READ, &pProps);
                if (pProps)
                {
                    std::string wasapiName = getDeviceName(pProps);

                    PROPVARIANT varFormFactor;
                    PropVariantInit(&varFormFactor);
                    pProps->GetValue(PKEY_AudioEndpoint_FormFactor, &varFormFactor);
                    UINT formFactor = varFormFactor.uintVal;
                    PropVariantClear(&varFormFactor);

                    bool isMic = (formFactor == Microphone ||
                                  formFactor == Headset ||
                                  formFactor == Handset);

                    // Skip mic-type devices — already added from DirectShow above
                    if (!isMic)
                    {
                        // For loopback capture devices (e.g. Stereo Mix), find
                        // the matching DShow name since it may also differ.
                        std::string deviceId = wasapiName;
                        for (const auto& dn : dshowNames)
                        {
                            if (dn == wasapiName ||
                                dn.find(wasapiName) != std::string::npos ||
                                wasapiName.find(dn) != std::string::npos)
                            {
                                deviceId = dn;
                                break;
                            }
                        }

                        AudioDeviceInfo info;
                        info.displayName = wasapiName;
                        info.id = deviceId;
                        info.isInput = false;
                        info.deviceType = AudioDeviceType::LoopbackCapture;
                        devices.push_back(std::move(info));
                    }

                    pProps->Release();
                }
                pDevice->Release();
            }
        }
        pCaptureCollection->Release();
    }

    // ── 2) eRender: Output devices usable via WASAPI loopback ──
    IMMDeviceCollection* pRenderCollection = nullptr;
    hr = pEnumerator->EnumAudioEndpoints(eRender, DEVICE_STATE_ACTIVE, &pRenderCollection);
    if (SUCCEEDED(hr) && pRenderCollection)
    {
        UINT count = 0;
        pRenderCollection->GetCount(&count);
        for (UINT i = 0; i < count; i++)
        {
            IMMDevice* pDevice = nullptr;
            if (SUCCEEDED(pRenderCollection->Item(i, &pDevice)) && pDevice)
            {
                IPropertyStore* pProps = nullptr;
                pDevice->OpenPropertyStore(STGM_READ, &pProps);
                if (pProps)
                {
                    std::string name = getDeviceName(pProps);
                    std::string endpointId = getEndpointId(pDevice);

                    AudioDeviceInfo info;
                    info.displayName = name;
                    info.id = endpointId;
                    info.isInput = false;
                    info.deviceType = AudioDeviceType::OutputDevice;
                    devices.push_back(std::move(info));

                    pProps->Release();
                }
                pDevice->Release();
            }
        }
        pRenderCollection->Release();
    }

    pEnumerator->Release();
    if (needUninit) CoUninitialize();
    return devices;
}

// ============================================================================
// GIF Conversion
// ============================================================================

bool ScreenRecorder_Win::convertToGif(const std::string& ffmpegPath,
                                      const std::string& inputPath,
                                      const std::string& outputPath,
                                      int fps)
{
    if (ffmpegPath.empty() || inputPath.empty() || outputPath.empty())
    {
        SRU_LOG_ERROR("convertToGif: empty path argument");
        return false;
    }

    std::string filterComplex = "fps=" + std::to_string(fps)
        + ",scale=640:-1:flags=lanczos,split[s0][s1];[s0]palettegen[p];[s1][p]paletteuse";

    ucf::utilities::ProcessBridgeConfig config;
    config.executablePath = ffmpegPath;
    config.arguments = {"-y", "-i", inputPath, "-filter_complex", filterComplex, outputPath};
    config.stopTimeoutMs = 120000;

    auto result = ucf::utilities::IProcessBridge::run(config);

    if (result.timedOut)
    {
        SRU_LOG_ERROR("convertToGif: FFmpeg timed out");
        return false;
    }

    std::error_code ec;
    bool ok = result.exitCode == 0 && std::filesystem::is_regular_file(outputPath, ec);
    if (!ok)
    {
        SRU_LOG_ERROR("convertToGif: ffmpeg exited with code " << result.exitCode);
    }
    return ok;
}

} // namespace ucf::utilities::screenrecording

#endif // _WIN32
