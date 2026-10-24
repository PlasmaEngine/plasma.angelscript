using PlasmaBuild.Core.Configuration;
using PlasmaBuild.Core.Rules;

public class EditorPluginAngelScriptModule : ModuleRules
{
    public EditorPluginAngelScriptModule(BuildContext context) : base(context)
    {
        PlasmaPackageSdk.ConfigurePackageModule(this, context, "Source/EditorPluginAngelScript",
            "EditorPluginAngelScriptPCH.h", "BUILDSYSTEM_BUILDING_EDITORPLUGINANGELSCRIPT_LIB");

        PlasmaPackageSdk.ConfigureEditorSdkConsumer(this, context);

        // The asset document type here builds on EditorPluginAssets, from the plasma.assets
        // built-in. Declared as a Requires in the manifest so the store can reason about it.
        PlasmaPackageSdk.ConfigureAssetsBuiltIn(this, context);

        PlasmaPackageSdk.AddPackagePluginImport(this, context, "Source/AngelScriptPlugin", "plAngelScriptPlugin");
        PlasmaPackageSdk.AddPackagePluginImport(this, context, "Source/EnginePluginAngelScript", "plEnginePluginAngelScript");

        PublicDependencies.Add("AngelScriptModule");

        // The script view is an engine-rendered code area.
        PlasmaPackageSdk.AddImportLibrary(this, context, "plEditorUi");
        PlasmaPackageSdk.AddImportLibrary(this, context, "plGameUiPlugin");
    }
}
