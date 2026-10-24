using System;
using System.IO;
using System.Linq;
using PlasmaBuild.Core.Configuration;
using PlasmaBuild.Core.Rules;

public class AngelScriptModule : ModuleRules
{
    public AngelScriptModule(BuildContext context) : base(context)
    {
        Type = ModuleType.ThirdParty;
        ExportsSymbols = false;
        SourceDirectory = "ThirdParty/AngelScript_plasmabuild";

        PublicIncludePaths.Add("../AngelScript/include");
        PrivateIncludePaths.Add("../AngelScript/source");
        PublicDefinitions.Add("BUILDSYSTEM_ENABLE_ANGELSCRIPT_SUPPORT");
        PublicDefinitions.Add("ANGELSCRIPT_DLL_LIBRARY_IMPORT");

        PrivateDefinitions.Add("ANGELSCRIPT_EXPORT");

        // The native x64 call bridge is MASM, which every x64 MSVC toolchain assembles (Windows and the platforms built
        // the same way).
        if (PlasmaBuildAngelScript.UsesNativeCallBridge(context))
        {
            AdditionalLinkerFlags.Add($"\"{PlasmaBuildAngelScript.GetNativeCallObjectPath(context)}\"");
        }

        CppStandard = CppStandard.Cpp20;
        UseUnityBuild = false;
        TreatWarningsAsErrors = false;
    }
}

public static class PlasmaBuildAngelScript
{
    private const string NativeCallSourceRelativePath = "ThirdParty/AngelScript/source/as_callfunc_x64_msvc_asm.asm";

    public static string GetNativeCallObjectPath(BuildContext context)
    {
        return Path.Combine(context.ProjectRoot, "Intermediate", $"{context.Platform}_{context.Configuration}", "AngelScriptModule", "as_callfunc_x64_msvc_asm.obj");
    }

    /// \brief Whether the build links the MASM x64 call bridge (as_callfunc_x64_msvc_asm.asm).
    public static bool UsesNativeCallBridge(BuildContext context)
    {
        return PlasmaPackageSdk.UsesMsvcToolchain(context) && context.Architecture != TargetArchitecture.X86 && context.Architecture != TargetArchitecture.ARM64;
    }

    public static void AddNativeCallAssemblerStep(TargetRules rules, BuildContext context)
    {
        if (!UsesNativeCallBridge(context))
        {
            return;
        }

        var sourcePath = Path.Combine(context.ProjectRoot, NativeCallSourceRelativePath);
        var outputPath = GetNativeCallObjectPath(context);
        var outputDirectory = Path.GetDirectoryName(outputPath);
        if (!string.IsNullOrWhiteSpace(outputDirectory))
        {
            Directory.CreateDirectory(outputDirectory);
        }

        if (rules.PreBuildSteps.Any(step => step.Outputs.Contains(outputPath, StringComparer.OrdinalIgnoreCase)))
        {
            return;
        }

        rules.PreBuildSteps.Add(new BuildStep
        {
            Description = "Assemble AngelScript native x64 call bridge",
            Command = $"{Quote(FindMsvcExecutableForRules("ml64.exe"))} /nologo /c /Fo{Quote(outputPath)} {Quote(sourcePath)}",
            WorkingDirectory = context.ProjectRoot,
            Outputs = { outputPath }
        });
    }

    private static string FindMsvcExecutableForRules(string executableName)
    {
        var vcToolsInstallDir = Environment.GetEnvironmentVariable("VCToolsInstallDir");
        if (!string.IsNullOrWhiteSpace(vcToolsInstallDir))
        {
            var candidate = Path.Combine(vcToolsInstallDir, "bin", "Hostx64", "x64", executableName);
            if (File.Exists(candidate))
            {
                return candidate;
            }
        }

        var pathValue = Environment.GetEnvironmentVariable("PATH") ?? string.Empty;

        foreach (var pathEntry in pathValue.Split(Path.PathSeparator, StringSplitOptions.RemoveEmptyEntries))
        {
            var candidate = Path.Combine(pathEntry.Trim('"'), executableName);
            if (File.Exists(candidate) && candidate.Contains(Path.Combine("VC", "Tools", "MSVC"), StringComparison.OrdinalIgnoreCase))
            {
                return candidate;
            }
        }

        foreach (var pathEntry in pathValue.Split(Path.PathSeparator, StringSplitOptions.RemoveEmptyEntries))
        {
            var candidate = Path.Combine(pathEntry.Trim('"'), executableName);
            if (File.Exists(candidate))
            {
                return candidate;
            }
        }

        foreach (var visualStudioRoot in EnumerateVisualStudioRoots())
        {
            var vcToolsRoot = Path.Combine(visualStudioRoot, "VC", "Tools", "MSVC");
            if (!Directory.Exists(vcToolsRoot))
            {
                continue;
            }

            foreach (var versionDir in Directory.EnumerateDirectories(vcToolsRoot).OrderByDescending(Path.GetFileName))
            {
                var candidate = Path.Combine(versionDir, "bin", "Hostx64", "x64", executableName);
                if (File.Exists(candidate))
                {
                    return candidate;
                }
            }
        }

        return executableName;
    }

    private static string[] EnumerateVisualStudioRoots()
    {
        var roots = new[]
        {
            Environment.GetFolderPath(Environment.SpecialFolder.ProgramFiles),
            Environment.GetFolderPath(Environment.SpecialFolder.ProgramFilesX86)
        };

        return roots
            .Where(root => !string.IsNullOrWhiteSpace(root))
            .Select(root => Path.Combine(root, "Microsoft Visual Studio"))
            .Where(Directory.Exists)
            .SelectMany(root => Directory.EnumerateDirectories(root).SelectMany(Directory.EnumerateDirectories))
            .ToArray();
    }

    private static string Quote(string path)
    {
        return $"\"{path}\"";
    }
}
