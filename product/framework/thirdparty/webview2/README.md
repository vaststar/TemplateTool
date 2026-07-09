# WebView2 SDK (vendored)

Vendored subset of the [`Microsoft.Web.WebView2`](https://www.nuget.org/packages/Microsoft.Web.WebView2) NuGet package.

- **Version:** 1.0.4078.44
- **Contents:**
  - `webview2/include/WebView2.h`, `webview2/include/WebView2EnvironmentOptions.h` — COM interface headers
  - `webview2/x64|arm64|x86/WebView2LoaderStatic.lib` — static loader (one per architecture)

## Why only these files

We link the **static** loader (`WebView2LoaderStatic.lib`), so there is **no
`WebView2Loader.dll` to ship**. The loader's only job is to locate and load the
system-installed *Evergreen* WebView2 runtime at run time. That runtime is **not
bundled** — it ships with Windows 11 and with Microsoft Edge on Windows 10, and
auto-updates independently.

## Updating

```powershell
$ver = "<new-version>"
$tmp = Join-Path $env:TEMP "wv2sdk"; Remove-Item -Recurse -Force $tmp -EA SilentlyContinue
Invoke-WebRequest "https://api.nuget.org/v3-flatcontainer/microsoft.web.webview2/$ver/microsoft.web.webview2.$ver.nupkg" -OutFile "$tmp.zip"
Expand-Archive "$tmp.zip" $tmp -Force
Copy-Item "$tmp\build\native\include\*.h" webview2\include\ -Force
foreach ($a in "x64","arm64","x86") { Copy-Item "$tmp\build\native\$a\WebView2LoaderStatic.lib" "webview2\$a\" -Force }
```

The CMake target `webview2` (INTERFACE) is defined in `webview2/CMakeLists.txt`
and is only added on Windows.
