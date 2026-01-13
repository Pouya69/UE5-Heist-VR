// Fill out your copyright notice in the Description page of Project Settings.

using UnrealBuildTool;
using System.Collections.Generic;

public class MetaXRTEST_01EditorTarget : TargetRules
{
	public MetaXRTEST_01EditorTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Editor;
		DefaultBuildSettings = BuildSettingsVersion.V5;

		ExtraModuleNames.AddRange( new string[] { "MetaXRTEST_01" } );
		
		IncludeOrderVersion = EngineIncludeOrderVersion.Unreal5_6;
	}
}
