// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class GardenManiac : ModuleRules
{
	public GardenManiac(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] {
			"Core",
			"CoreUObject",
			"Engine",
			"InputCore",
			"EnhancedInput",
			"AIModule",
			"StateTreeModule",
			"GameplayStateTreeModule",
			"UMG",
			"Slate"
		});

		PrivateDependencyModuleNames.AddRange(new string[] { "ApplicationCore" });

		PublicIncludePaths.AddRange(new string[] {
			"GardenManiac",
			"GardenManiac/Variant_Horror",
			"GardenManiac/Variant_Horror/UI",
			"GardenManiac/Variant_Shooter",
			"GardenManiac/Variant_Shooter/AI",
			"GardenManiac/Variant_Shooter/UI",
			"GardenManiac/Variant_Shooter/Weapons"
		});

		// Uncomment if you are using Slate UI
		// PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });

		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
	}
}
