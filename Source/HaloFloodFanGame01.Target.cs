// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;
using System.Collections.Generic;

public class HaloFloodFanGame01Target : TargetRules
{
	public HaloFloodFanGame01Target(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Game;
		DefaultBuildSettings = BuildSettingsVersion.Latest;
		IncludeOrderVersion = EngineIncludeOrderVersion.Latest ;
		ExtraModuleNames.Add("HaloFloodFanGame01");
	}
}
