using PlasmaBuild.Core.Configuration;
using PlasmaBuild.Core.Rules;

public class AngelScriptPluginTarget : TargetRules
{
    public AngelScriptPluginTarget(BuildContext context) : base(context)
    {
        Type = TargetType.SharedLibrary;
        OutputName = "plAngelScriptPlugin";
        OutputDirectory = PlasmaPackageSdk.PackageBinaryDirectory(context);

        UsePCHFiles = true;
        UseUnityBuild = true;
        UseAdaptiveUnityBuild = true;
        UseIncrementalLinking = true;

        ExtraModules.Add("AngelScriptPluginModule");

        // Compiled in rather than imported - there is no standalone AngelScript DLL.
        ExtraModules.Add("AngelScriptModule");

        // AngelScript's x64 native calling convention bridge is hand-written assembly, so it
        // needs ml64 before the link. Without this the plugin fails to link on x64.
        PlasmaBuildAngelScript.AddNativeCallAssemblerStep(this, context);
    }
}
