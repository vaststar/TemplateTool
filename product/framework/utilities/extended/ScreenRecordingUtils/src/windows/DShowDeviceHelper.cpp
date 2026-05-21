#include "DShowDeviceHelper.h"

#ifdef _WIN32

#include "LoggerDefine.h"
#include "WinEncodingUtils.h"

#include <initguid.h>
#include <mmdeviceapi.h>
#include <functiondiscoverykeys_devpkey.h>
#include <dshow.h>

namespace ucf::utilities::screenrecording {

std::string escapeDshowDeviceName(const std::string& name)
{
    std::string escaped;
    escaped.reserve(name.size() + 8);
    for (char c : name)
    {
        if (c == '\\' || c == '\'' || c == ';' || c == '=' || c == ':')
            escaped += '\\';
        escaped += c;
    }
    return escaped;
}

std::vector<DShowDeviceEntry> enumerateDShowCaptureDevices()
{
    std::vector<DShowDeviceEntry> entries;

    ICreateDevEnum* pDevEnum = nullptr;
    HRESULT hr = CoCreateInstance(CLSID_SystemDeviceEnum, nullptr, CLSCTX_INPROC_SERVER,
                                  IID_ICreateDevEnum, reinterpret_cast<void**>(&pDevEnum));
    if (FAILED(hr) || !pDevEnum) return entries;

    IEnumMoniker* pEnum = nullptr;
    hr = pDevEnum->CreateClassEnumerator(CLSID_AudioInputDeviceCategory, &pEnum, 0);
    if (hr == S_OK && pEnum)
    {
        IMoniker* pMoniker = nullptr;
        while (pEnum->Next(1, &pMoniker, nullptr) == S_OK)
        {
            DShowDeviceEntry entry;

            IPropertyBag* pPropBag = nullptr;
            if (SUCCEEDED(pMoniker->BindToStorage(nullptr, nullptr, IID_IPropertyBag,
                                                   reinterpret_cast<void**>(&pPropBag))))
            {
                VARIANT var;
                VariantInit(&var);
                if (SUCCEEDED(pPropBag->Read(L"FriendlyName", &var, nullptr))
                    && var.vt == VT_BSTR && var.bstrVal)
                    entry.friendlyName = wideToUtf8(var.bstrVal);
                VariantClear(&var);
                pPropBag->Release();
            }

            IBindCtx* pBindCtx = nullptr;
            if (SUCCEEDED(CreateBindCtx(0, &pBindCtx)))
            {
                LPOLESTR pDisplayName = nullptr;
                if (SUCCEEDED(pMoniker->GetDisplayName(pBindCtx, nullptr, &pDisplayName))
                    && pDisplayName)
                {
                    entry.monikerName = wideToUtf8(pDisplayName);
                    CoTaskMemFree(pDisplayName);
                }
                pBindCtx->Release();
            }

            if (entry.monikerName.empty())
                entry.monikerName = entry.friendlyName;

            if (!entry.friendlyName.empty())
            {
                SRU_LOG_DEBUG("DShow device: friendly='" << entry.friendlyName
                              << "' moniker='" << entry.monikerName << "'");
                entries.push_back(std::move(entry));
            }
            pMoniker->Release();
        }
        pEnum->Release();
    }
    pDevEnum->Release();
    return entries;
}

bool isDShowCompatible(const std::string& id)
{
    if (id.empty()) return true;
    if (id.rfind("@device", 0) == 0) return true;
    if (id.front() == '{' && id.find("}.") != std::string::npos) return false;
    return true;
}

std::string getWasapiFriendlyName(const std::string& endpointId)
{
    IMMDeviceEnumerator* pEnumerator = nullptr;
    HRESULT hr = CoCreateInstance(__uuidof(MMDeviceEnumerator), nullptr,
                                  CLSCTX_ALL, __uuidof(IMMDeviceEnumerator),
                                  reinterpret_cast<void**>(&pEnumerator));
    if (FAILED(hr) || !pEnumerator) return {};

    auto wideId = utf8ToWide(endpointId);
    IMMDevice* pDevice = nullptr;
    hr = pEnumerator->GetDevice(wideId.data(), &pDevice);
    if (FAILED(hr) || !pDevice) { pEnumerator->Release(); return {}; }

    std::string name;
    IPropertyStore* pProps = nullptr;
    if (SUCCEEDED(pDevice->OpenPropertyStore(STGM_READ, &pProps)) && pProps)
    {
        PROPVARIANT varName;
        PropVariantInit(&varName);
        if (SUCCEEDED(pProps->GetValue(PKEY_Device_FriendlyName, &varName))
            && varName.vt == VT_LPWSTR)
            name = wideToUtf8(varName.pwszVal);
        PropVariantClear(&varName);
        pProps->Release();
    }
    pDevice->Release();
    pEnumerator->Release();
    return name;
}

std::string validateDShowDeviceId(const std::string& deviceId, const char* label)
{
    if (isDShowCompatible(deviceId))
        return deviceId;

    SRU_LOG_WARN(label << " device is a WASAPI endpoint ID: " << deviceId);

    std::string wasapiName = getWasapiFriendlyName(deviceId);
    if (wasapiName.empty())
    {
        SRU_LOG_WARN(label << " could not resolve WASAPI endpoint to friendly name");
        return {};
    }
    SRU_LOG_INFO(label << " WASAPI endpoint resolves to: " << wasapiName);

    auto dshowDevices = enumerateDShowCaptureDevices();

    for (const auto& dev : dshowDevices)
    {
        if (dev.friendlyName == wasapiName && !dev.monikerName.empty())
            return dev.monikerName;
    }

    for (const auto& dev : dshowDevices)
    {
        if (!dev.friendlyName.empty() && !dev.monikerName.empty() &&
            (dev.friendlyName.find(wasapiName) != std::string::npos ||
             wasapiName.find(dev.friendlyName) != std::string::npos))
            return dev.monikerName;
    }

    SRU_LOG_WARN(label << " no DShow device matches '" << wasapiName << "'");
    return {};
}

std::string monikerToFriendlyName(const std::string& moniker)
{
    auto devices = enumerateDShowCaptureDevices();
    for (const auto& dev : devices)
    {
        if (dev.monikerName == moniker && !dev.friendlyName.empty())
            return dev.friendlyName;
    }
    return {};
}

std::string formatDShowAudioInput(const std::string& deviceId, const char* label)
{
    std::string resolved = validateDShowDeviceId(deviceId, label);
    if (!resolved.empty())
    {
        // If resolved to a moniker, look up the friendly name for FFmpeg
        if (resolved.rfind("@device", 0) == 0)
        {
            std::string friendly = monikerToFriendlyName(resolved);
            if (!friendly.empty())
            {
                SRU_LOG_DEBUG(label << " moniker resolved to friendly name: " << friendly);
                return escapeDshowDeviceName(friendly);
            }
            // Last resort: pass the moniker directly and hope FFmpeg supports it
            SRU_LOG_WARN(label << " could not resolve moniker to friendly name, using raw moniker");
            return resolved;
        }
        return escapeDshowDeviceName(resolved);
    }

    // deviceId was empty or unresolvable - pick the first available DShow device
    auto devices = enumerateDShowCaptureDevices();
    if (!devices.empty())
    {
        SRU_LOG_INFO(label << " fallback to first DShow device: " << devices[0].friendlyName);
        return escapeDshowDeviceName(devices[0].friendlyName);
    }

    SRU_LOG_WARN(label << " no DShow capture devices found");
    return {};
}

} // namespace ucf::utilities::screenrecording

#endif // _WIN32
