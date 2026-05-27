// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class BuildingFramework : ModuleRules
{
    public BuildingFramework(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicIncludePaths.AddRange(
            new string[]
            {
            }
        );

        PrivateIncludePaths.AddRange(
            new string[]
            {
            }
        );

        PublicDependencyModuleNames.AddRange(
            new string[]
            {
				// Core
				"Core",

				// Unreal Core Systems
				"CoreUObject",
                "Engine",

				// UI
				"Slate",
                "SlateCore",
                "UMG",

				// Networking
				"NetCore",
                "Networking",

				// Gameplay Tags 
				"GameplayTags",
            }
        );

        PrivateDependencyModuleNames.AddRange(
            new string[]
            {
            }
        );

        DynamicallyLoadedModuleNames.AddRange(
            new string[]
            {
				// Steam/EOS/etc later
			}
        );
    }
}