// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class Ninth_baseballGame : ModuleRules
{
	public Ninth_baseballGame(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
	
		PublicDependencyModuleNames.AddRange(new string[]
		{
			// Initial Dependencies
			"Core", "CoreUObject", "Engine", "InputCore", "EnhancedInput",
			
			// UI
			"UMG", "Slate", "SlateCore", 
			
		});

		PrivateDependencyModuleNames.AddRange(new string[] {  });
	}
}
