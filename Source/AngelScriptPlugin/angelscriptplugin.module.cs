using PlasmaBuild.Core.Configuration;
using PlasmaBuild.Core.Rules;

public class AngelScriptPluginModule : ModuleRules
{
    public AngelScriptPluginModule(BuildContext context) : base(context)
    {
        PlasmaPackageSdk.ConfigurePackageModule(this, context, "Source/AngelScriptPlugin",
            "AngelScriptPluginPCH.h", "BUILDSYSTEM_BUILDING_ANGELSCRIPTPLUGIN_LIB");

        // The AngelScript library itself, vendored in this package rather than in the SDK.
        PublicDependencies.Add("AngelScriptModule");
    }
}
