// #pl-version 1

using PlasmaBuild.Core.Configuration;
using PlasmaBuild.Core.Rules;

public class PlasmaAngelScriptWorkspace : WorkspaceRules
{
    public PlasmaAngelScriptWorkspace(BuildContext context) : base(context)
    {
        TargetNames.Add("AngelScriptPlugin");
        TargetNames.Add("EnginePluginAngelScript");
        TargetNames.Add("EditorPluginAngelScript");
    }
}
