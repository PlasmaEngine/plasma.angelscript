using PlasmaBuild.Core.Configuration;
using PlasmaBuild.Core.Rules;

public class EnginePluginAngelScriptModule : ModuleRules
{
    public EnginePluginAngelScriptModule(BuildContext context) : base(context)
    {
        PlasmaPackageSdk.ConfigurePackageModule(this, context, "Source/EnginePluginAngelScript",
            "EnginePluginAngelScriptPCH.h", "BUILDSYSTEM_BUILDING_ENGINEPLUGINANGELSCRIPT_LIB");

        PlasmaPackageSdk.ConfigureEditorSdkConsumer(this, context);
        PlasmaPackageSdk.AddPackagePluginImport(this, context, "Source/AngelScriptPlugin", "plAngelScriptPlugin");

        PublicDependencies.Add("AngelScriptModule");
    }
}
