using PlasmaBuild.Core.Configuration;
using PlasmaBuild.Core.Rules;

public class EnginePluginAngelScriptTarget : TargetRules
{
    public EnginePluginAngelScriptTarget(BuildContext context) : base(context)
    {
        Type = TargetType.SharedLibrary;
        OutputName = "plEnginePluginAngelScript";
        OutputDirectory = PlasmaPackageSdk.PackageBinaryDirectory(context);

        UsePCHFiles = true;
        UseUnityBuild = true;
        UseAdaptiveUnityBuild = true;
        UseIncrementalLinking = true;

        TargetDependencies.Add("AngelScriptPlugin");

        ExtraModules.Add("EnginePluginAngelScriptModule");
        ExtraModules.Add("AngelScriptModule");
        PlasmaBuildAngelScript.AddNativeCallAssemblerStep(this, context);
    }
}
