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
- `UtilModule/internal/`: implementation details for public utility modules.
- `VersionModule/`: version metadata and generated application information.

## Public entry points

- `BuildModule(...)`: non-Qt static or shared C++ library.
- `BuildQtModule(...)`: Qt library, optionally with a QML module.
- `BuildInterface(...)`: header-only interface library.
- `BuildUnitTestModule(...)`: Catch2 test executable; links
  `Catch2::Catch2WithMain` by default.
- `BuildInstallModule(...)`: common install/export rules.
- `BuildRCFileModule(...)`: Windows version resource generation.
- `BuildBundlePListModule(...)`: macOS bundle property-list generation.
- `BuildLinuxDesktopModule(...)`: Linux desktop-entry generation.
- `generate_from_template(...)`: Jinja-based generated source/resource file.
- `generate_app_version_meta(...)`: version metadata generated from product
  information and the current Git state.
- `generate_app_info_files(...)`: platform metadata generated from the shared
  version metadata file.
- `generate_app_version_header(...)`: semantic adapter for generating the
  public application-version header.
- `get_required_target_property(...)`: reads required configuration metadata
  published by another target and fails when the target or property is absent.
- `validate_asset_tokens(...)`: validates asset token/theme consistency and
  referenced files.
- `validate_color_tokens(...)`: validates color token/theme consistency.
- `validate_font_tokens(...)`: validates font token/theme/palette consistency.

Callers must include the module that owns the public function they use. Public
modules must directly include every helper module they call; relying on include
order from another directory is not supported.

## Argument conventions

- Boolean switches are options such as `STATIC_LIB`, `SHARED_LIB`, and
  `NO_INSTALL`. `STATIC_LIB` and `SHARED_LIB` are mutually exclusive; omitting
  both retains the historical shared-library default.
- Single values use names such as `MODULE_NAME` and `IDE_FOLDER`.
- Lists use plural nouns where possible.
- Target usage requirements retain their CMake visibility in the argument
  name: `PUBLIC`, `PRIVATE`, or `INTERFACE`.
- Compile definitions must use `TARGET_PUBLIC_DEFINITIONS` or
  `TARGET_PRIVATE_DEFINITIONS`. The ambiguous historical
  `TARGET_DEFINITIONS` argument is not supported.
- `BuildUnitTestModule` owns the default Catch2 main dependency. Use
  `NO_DEFAULT_CATCH_MAIN` only when the test supplies its own main, and link
  any replacement Catch2 target through `TARGET_ADD_LINK_LIBRARY_PRIVATE`.
- Unknown arguments are configuration errors in new or revised APIs.
- Required target names, files, templates, and output variables are validated
  at the public function boundary.

Existing public argument names are kept for compatibility. New APIs should
prefer explicit names such as `PRIVATE_LINK_LIBRARIES`,
`PUBLIC_COMPILE_DEFINITIONS`, and `OUTPUT_TARGET_VAR`.

## Template generation

`generate_from_template(...)` requires an explicit output variable name:

```cmake
generate_from_template(
    TEMPLATE_FILE "${template_file}"
    INPUT_FILE "${input_file}"
    OUTPUT_FILE "${output_file}"
    OUTPUT_TARGET_VAR generated_target
)
```

The function writes the generated target name to `generated_target` in the
caller's scope. The target name is derived from the normalized output path, so
same-named files in different directories do not collide. Re-registering the
same output with a different template-generation configuration is an error.

The shared `jinja_venv` runtime is registered once per build tree. Generated
targets depend on both that runtime and its requirements file; changing the
requirements rebuilds the runtime and regenerates affected outputs.

## Token validation

Token validators receive the exact theme files used for generation and return
an explicit target name:

```cmake
validate_asset_tokens(
    SCRIPT "${validator_script}"
    TOKEN_FILE "${token_file}"
    THEME_FILES ${theme_files}
    ASSET_ROOT "${asset_root}"
    OUTPUT_TARGET_VAR validation_target
)
```

`THEME_FILES` is intentionally explicit; validators must not discover a
different input set with `file(GLOB)`. The returned target is not independently
part of `ALL`. Generated resource targets and their output rules depend on it,
so validation runs once when that resource graph is requested and completes
before parallel template generation starts.

Validation targets are intentionally always-run. In particular, asset
validation must detect a referenced file being deleted even when the token and
theme JSON timestamps did not change. Successful validation does not by itself
rewrite generated files, so an incremental build still preserves generated
output timestamps.

## Target metadata

Cross-directory build metadata belongs to the target that publishes it, not in
global cache variables. Consumers should retrieve required metadata explicitly:

```cmake
get_required_target_property(
    TARGET ResourceAsset
    PROPERTY UCF_RESOURCE_TOKEN_FILE
    OUTPUT_VARIABLE asset_token_file
)
```

Target properties transfer configuration-time values only. Link relationships
and generated-file ordering must still be expressed through target links and
dependencies.

## Version generation

Version generation follows this build-time chain:

```text
Git state -> git_info.meta -> version_meta.json
                            -> appVersion.h / plist / RC / desktop / CPack
```

Every generated output rule depends directly on both its upstream generated
file and the target that produces that file. The generated wrapper target also
depends on the upstream target. This two-level dependency is required so a
consumer that directly requests an output file cannot bypass prerequisite
generation during a clean parallel build.

Git metadata is refreshed whenever the version target is requested, but its
file is rewritten only when the Git hash, branch, or commit depth changes. This
keeps Git state detection correct without forcing downstream recompilation on
every build.

Generated version targets store their normalized output and configuration
signature as target properties. Identical registrations reuse the existing
target; conflicting registrations for the same output are configuration
errors.

## Public versus internal functions

Public functions use their documented names. Internal helpers use the `_tt_`
prefix and may change without migrating project `CMakeLists.txt` files.

Do not call functions from an `internal/` directory outside the owning public
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

Public API migrations should be incremental. Keep an existing entry point as
an adapter until all call sites have moved, then remove the obsolete argument
or entry point so new code cannot continue using it.
