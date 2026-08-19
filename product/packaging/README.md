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
