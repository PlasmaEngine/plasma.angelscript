# plasma.angelscript

AngelScript scripting for PlasmaEngine — the runtime, the editor engine process side, and an editor
asset type for authoring scripts. Moved out of the engine tree to be built and distributed as a
package.

Published as [`plasma.angelscript`](https://plasmaengine.github.io/PlasmaPackages/index.plPackageIndex)
in the Plasma package registry.

## Layout

```
Package.plPackage           the manifest - identity, compatibility, dependencies, components
AngelScript.plPluginBundle  legacy descriptor, kept until the engine reads manifests directly
Source/
  AngelScriptPlugin/        runtime               -> plAngelScriptPlugin
  EnginePluginAngelScript/  editor engine process -> plEnginePluginAngelScript
  EditorPluginAngelScript/  editor                -> plEditorPluginAngelScript  (script asset, editor window)
EditorIcons/                editor icons, registered as an icon folder by the editor plugin
ThirdParty/
  AngelScript/              the upstream library, compiled into plAngelScriptPlugin
  AngelScript_plasmabuild/  the unity source list PlasmaBuild compiles
Tests/                      the AngelScript test suite, moved here with the code it exercises
Configs/                    build rules for building against an installed engine SDK
Branding/                   icon and banner
```

AngelScript is statically folded into `plAngelScriptPlugin` rather than shipped as its own DLL,
which is how the engine built it too — there was never a standalone AngelScript DLL in `Binaries/`.
That is why `PackageDependencies` is empty.

## Dependencies

- `plasma.assets ^1.0` — `plEditorPluginAngelScript` defines an asset document type, so it builds on
  `EditorPluginAssets` and must load after it. `plasma.assets` is a built-in that ships with the
  engine, but it is declared like anything else so the store can reason about it.

## Building

Requires an engine SDK — a built engine checkout — named by `PLASMA_SDK_ROOT`:

```bash
PLASMA_SDK_ROOT=/path/to/engine plasmabuild build EditorPluginAngelScript
```

Each of the three targets builds separately. Output lands in `Bin/Windows_x64_Development/`.

On x64 the build shells out to `ml64.exe` to assemble AngelScript's native calling-convention
bridge before linking, so an MSVC toolchain must be on `PATH` or `VCToolsInstallDir` must be set.
Without it the plugin fails to link with unresolved call-bridge symbols.

## Tests

`Tests/` holds the suite that used to live in the engine's `GameEngineTest`. It is source only —
there is no package test runner yet, so nothing builds or executes it. It moved here rather than
being deleted because it tests this package's code, and it exercises the AngelScript API directly
(`plAngelScriptEngineSingleton`, `asIScriptContext`), so it cannot run without the plugin present.

## Publishing a new version

Staging and publishing are driven from the engine and registry repositories:

```bash
# engine repo - stage and validate, emitting machine-readable output
python Utilities/PackageInspector.py <staged-package-dir> --json <out>.json

# registry repo - upload blobs and record the release
python tools/publish_package.py <out>.json
```

Versions are immutable. A mistake is fixed by publishing a new patch version, never by replacing an
existing one — every lock file that already trusts a version must keep resolving to the same bytes.
