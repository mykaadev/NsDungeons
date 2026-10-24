// Copyright (C) 2025 nulled.softworks. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "NsDungeonGenerator.h"
#include "NsDungeonLibrary.generated.h"

/** Blueprint calls of NsDungeons: make a dungeon from a rule set, and read its cells */
UCLASS()
class NSDUNGEONS_API UNsDungeonLibrary : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()

public:
    /** Makes a dungeon from a rule set and a seed. With no rule set, it uses the default settings */
    UFUNCTION(BlueprintCallable, Category = "NsDungeons")
    static FNsDungeonResult GenerateDungeon(const UNsDungeonRuleSet* RuleSet, int32 Seed, bool bKeepSteps = false);

    /** Returns the kind of a cell of a layout */
    UFUNCTION(BlueprintPure, Category = "NsDungeons")
    static ENsDungeonCell GetLayoutCell(const FNsDungeonLayout& Layout, FIntPoint Cell);

    /** Returns the middle of a cell in local space, with a cell size in cm. Cell (0, 0) has its middle at half a cell from the origin */
    UFUNCTION(BlueprintPure, Category = "NsDungeons")
    static FVector CellToLocal(FIntPoint Cell, float CellSize = 100.0f);

    /** Returns the cell under a point in local space, with a cell size in cm */
    UFUNCTION(BlueprintPure, Category = "NsDungeons")
    static FIntPoint LocalToCell(const FVector& Local, float CellSize = 100.0f);
};
