# Application packaging

- `runtime/` defines and applies the install-tree layout and platform
  deployment steps to the already-created application target.
- `distribution/` configures CPack and release metadata from the completed
  install rules. It must be added after application targets.

Third-party directories remain independent of these project-owned APIs.
Opaque project runtime payloads are registered by their owning project module
and consumed only by `runtime/`.

`TemplateTool::Application` is the stable contract between the application and
packaging layers. The packaging root resolves its real target and output name;
runtime and distribution code do not depend on the concrete executable name.

`TemplateTool::QtDeploymentRoot` is the stable Qt runtime scanning contract.
It remains separate from the non-Qt application executable and resolves to the
Qt library passed to platform deployment tools.

After platform deployment, `runtime/audit/runtime_dependencies.cmake` scans
the installed executable, shared libraries, and dynamically loaded plugins.
It writes a categorized report under
`<build>/packaging/runtime-audit/<config>/` and warns about unresolved,
conflicting, or external non-system dependencies. The audit is report-only and
can be disabled with `TT_ENABLE_RUNTIME_DEPENDENCY_AUDIT=OFF`.

The release workflow prints each platform report in the job summary and keeps
it in a separate `runtime-dependency-audit-<platform>` artifact. Release
creation downloads only `package-*` artifacts, so diagnostic reports are not
published as release assets.

## Distribution layout

```text
distribution/
  CMakeLists.txt
  common/
    cpack_runtime_pre_build.cmake.in
    package-info.json.in
  windows/
    packaging.cmake
    cpack_windows_config.cmake.in
    nsis/
      policy.nsh
      template.cmake
  macos/
    packaging.cmake
    cpack_macos_config.cmake.in
  linux/
    packaging.cmake
    cpack_linux_config.cmake.in
    cpack_linux_pre_build.cmake.in
  release-config.json
  scripts/
```

Each platform's `packaging.cmake` is its entry point. It selects the platform
CPack configuration template and prepares platform-specific helpers. Windows
also connects the NSIS installation policy and CPack template adapter; Linux
generates its desktop, icon and launcher integration helper.

The distribution root includes the selected platform entry, generates the
shared runtime pre-build script, calls `BuildCPackModule` once, generates
`package-info.json`, and finally includes CPack. Platform helpers are prepared
first so their paths are available when configuring the shared script.
Platform entries locate source files with `CMAKE_CURRENT_LIST_DIR`; generated
files remain in the distribution root's binary directory.

The shared pre-build script prepares CPack's private staging tree after runtime
deployment. It keeps the normal install tree separate from package-specific
layout changes. Release configuration and manifest generation remain shared
across platforms.

The generated platform configuration filenames and
`<distribution-build-dir>/package-info.json` location stay stable for build and
release consumers. Package formats continue to be selected by the existing
package presets.
