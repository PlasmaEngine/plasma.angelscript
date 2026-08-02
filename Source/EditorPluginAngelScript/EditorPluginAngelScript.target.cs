using PlasmaBuild.Core.Configuration;
using PlasmaBuild.Core.Rules;

public class EditorPluginAngelScriptTarget : TargetRules
{
    public EditorPluginAngelScriptTarget(BuildContext context) : base(context)
    {
        Type = TargetType.SharedLibrary;
        OutputName = "plEditorPluginAngelScript";
        OutputDirectory = PlasmaPackageSdk.PackageBinaryDirectory(context);

        UsePCHFiles = true;
        UseUnityBuild = true;
        UseAdaptiveUnityBuild = true;
        UseIncrementalLinking = true;

        TargetDependencies.Add("AngelScriptPlugin");
        TargetDependencies.Add("EnginePluginAngelScript");

        ExtraModules.Add("EditorPluginAngelScriptModule");
        ExtraModules.Add("AngelScriptModule");
        PlasmaBuildAngelScript.AddNativeCallAssemblerStep(this, context);
    }
}
