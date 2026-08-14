# TemplateTool CMake modules

This directory contains the project-owned CMake API. Application and library
`CMakeLists.txt` files should prefer these entry points over repeating target
setup logic.

## Directory responsibilities

- `BuildModule/`: target creation, usage requirements, installation, and
  platform-specific target metadata.
- `BuildModule/internal/`: implementation details used by the public build
  functions. Files and functions in this directory are not public APIs.
- `UtilModule/`: reusable platform, code-generation, and validation helpers.
- `VersionModule/`: version metadata and generated application information.

## Public entry points

- `BuildModule(...)`: non-Qt static or shared C++ library.
- `BuildQtModule(...)`: Qt library, optionally with a QML module.
- `BuildInterface(...)`: header-only interface library.
- `BuildUnitTestModule(...)`: Catch2 test executable.
- `BuildInstallModule(...)`: common install/export rules.
- `BuildRCFileModule(...)`: Windows version resource generation.
- `BuildBundlePListModule(...)`: macOS bundle property-list generation.
- `BuildLinuxDesktopModule(...)`: Linux desktop-entry generation.
- `generate_from_template(...)`: Jinja-based generated source/resource file.

Callers must include the module that owns the public function they use. Public
modules must directly include every helper module they call; relying on include
order from another directory is not supported.

## Argument conventions

- Boolean switches are options such as `STATIC_LIB` and `NO_INSTALL`.
- Single values use names such as `MODULE_NAME` and `IDE_FOLDER`.
- Lists use plural nouns where possible.
- Target usage requirements retain their CMake visibility in the argument
  name: `PUBLIC`, `PRIVATE`, or `INTERFACE`.
- Unknown arguments are configuration errors in new or revised APIs.
- Required target names, files, templates, and output variables are validated
  at the public function boundary.

Existing public argument names are kept for compatibility. New APIs should
prefer explicit names such as `PRIVATE_LINK_LIBRARIES`,
`PUBLIC_COMPILE_DEFINITIONS`, and `OUTPUT_TARGET_VAR`.

## Public versus internal functions

Public functions use their documented names. Internal helpers use the `_tt_`
prefix and may change without migrating project `CMakeLists.txt` files.

Do not call functions from `BuildModule/internal/` outside the owning public
module.

## Logging

Public functions may print one concise `STATUS` line during normal configure.
Detailed argument dumps belong behind `CMAKE_VERBOSE_MAKEFILE`. Fatal messages
should identify both the public function and the invalid argument or target.

## Validation

Changes to these modules should be checked with:

1. `git diff --check`.
2. A clean CMake configure for every affected platform when available.
3. At least one target for every changed public build function.
4. Inspection of generated compile/link commands when usage requirements or
   generated include directories change.

Public API migrations should be incremental. Keep the existing entry point as
an adapter until all call sites have moved to the replacement API.
