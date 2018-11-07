// Copyright 1998-2015 Epic Games, Inc. All Rights Reserved.

namespace UnrealBuildTool.Rules
{
	public class DirectInputPadPlugin : ModuleRules
	{
		public DirectInputPadPlugin(ReadOnlyTargetRules Target):base(Target)
		{
			PrivatePCHHeaderFile = "Private/DirectInputPadPluginPrivatePCH.h";
			
			PublicIncludePaths.AddRange(
				new string[] {
					// "DirectInputPadPlugin/Public",
					// ... add public include paths required here ...
				}
				);

			PrivateIncludePaths.AddRange(
				new string[] {
					// "DirectInputPadPlugin/Private",
					// ... add other private include paths required here ...
				}
				);

			PublicDependencyModuleNames.AddRange(
				new string[]
				{
                    "Core",
                    "CoreUObject",
		    "ApplicationCore",
                    "Engine",
                    "InputDevice",
					// ... add other public dependencies that you statically link with here ...
				}
				);

			PrivateDependencyModuleNames.AddRange(
				new string[]
				{
                    "Slate",
                    "SlateCore",
                    "InputCore",
					// ... add private dependencies that you statically link with here ...
				}
				);

            if (Target.bBuildEditor)
            {
                PrivateDependencyModuleNames.AddRange(new string[]{ "MainFrame", });
            }

            DynamicallyLoadedModuleNames.AddRange(
				new string[]
				{
					// ... add any modules that your module loads dynamically here ...
				}
				);
		}
	}
}
