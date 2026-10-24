// Copyright (C) 2025 nulled.softworks. All rights reserved.

using UnrealBuildTool;

public class NsDungeonsEditor : ModuleRules
{
    public NsDungeonsEditor(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(
            new string[]
            {
                "Core",
                "CoreUObject",
                "Engine",
            }
        );

        PrivateDependencyModuleNames.AddRange(
            new string[]
            {
                "NsDungeons",
                "Slate",
                "SlateCore",
                "UnrealEd",
            }
        );
    }
}
