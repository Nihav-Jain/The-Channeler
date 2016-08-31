// Copyright 2014 Tobii Technology AB. All Rights Reserved.

using System.IO;

namespace UnrealBuildTool.Rules
{
    public class TobiiEyeX : ModuleRules
    {
        private string ModulePath
        {
            get { return Path.GetDirectoryName(RulesCompiler.GetModuleFilename(this.GetType().Name)); }
        }

        private string ThirdPartyPath
        {
            get { return Path.GetFullPath(Path.Combine(ModulePath, "../../ThirdParty/")); }
        }

        public TobiiEyeX(TargetInfo Target)
        {
            MinFilesUsingPrecompiledHeaderOverride = 1;
            bFasterWithoutUnity = true;

            PublicIncludePaths.AddRange(
                new string[] {
                    "TobiiEyeX/Public"
					// ... add public include paths required here ...
				}
                );

            PrivateIncludePaths.AddRange(
                new string[] {
					"TobiiEyeX/Private",
                    Path.Combine(ThirdPartyPath, "EyeX", "include")
					// ... add other private include paths required here ...
				}
                );

            PublicDependencyModuleNames.AddRange(
                new string[]
				{
					"Core",
					"CoreUObject",
                    "Engine"
					// ... add other public dependencies that you statically link with here ...
				}
                );

            PrivateDependencyModuleNames.AddRange(
                new string[]
				{
					// ... add private dependencies that you statically link with here ...
				}
                );

            DynamicallyLoadedModuleNames.AddRange(
                new string[]
				{
					// ... add any modules that your module loads dynamically here ...
				}
                );
        }
    }
}