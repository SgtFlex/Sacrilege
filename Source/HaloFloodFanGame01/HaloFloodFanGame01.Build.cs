// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class HaloFloodFanGame01 : ModuleRules
{
	public HaloFloodFanGame01(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "HeadMountedDisplay", "EnhancedInput", "UMG" , "Niagara"});
	}
}
