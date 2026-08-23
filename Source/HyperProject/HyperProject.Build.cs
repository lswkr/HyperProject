// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class HyperProject : ModuleRules
{
	public HyperProject(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
	
		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "EnhancedInput", "Niagara", "AIModule", "GameplayAbilities", "UMG", "OnlineSubsystem" });

		PrivateDependencyModuleNames.AddRange(new string[] { "GameplayAbilities", "GameplayTasks", "GameplayTags", 
			"OnlineSubsystem", "OnlineSubsystemEOS", "OnlineSubsystemUtils", "Networking", "HTTP", "Json" });

		// Uncomment if you are using Slate UI
		// PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });
		
		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
	}
}
