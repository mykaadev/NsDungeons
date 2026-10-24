// Copyright (C) 2025 nulled.softworks. All rights reserved.

using UnrealBuildTool;

public class NsDungeons : ModuleRules
{
    public NsDungeons(ReadOnlyTargetRules Target) : base(Target)
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
    }
}
