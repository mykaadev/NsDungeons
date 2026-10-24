// Copyright (C) 2025 nulled.softworks. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "NsDungeonGraph.h"
#include "NsDungeonRuleSet.generated.h"

/** The weight of one major cycle type: a higher weight picks it more often */
USTRUCT(BlueprintType)
struct NSDUNGEONS_API FNsDungeonCycleWeight
{
    GENERATED_BODY()

    /** The cycle type */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "NsDungeons")
    ENsDungeonCycleType Type = ENsDungeonCycleType::LockAndKey;

    /** Its weight. 0 turns it off */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "NsDungeons", meta = (ClampMin = "0"))
    int32 Weight = 1;
};

/** The values of one generation. A rule set asset holds them, and code can make them too */
USTRUCT(BlueprintType)
struct NSDUNGEONS_API FNsDungeonSettings
{
    GENERATED_BODY()

    /** The size of the node grid. Each node can become a room */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Grid", meta = (ClampMin = "3", ClampMax = "16"))
    FIntPoint GridSize = FIntPoint(5, 5);

    /** The smallest grid distance (steps along the grid) from the start to the goal */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Grid", meta = (ClampMin = "2"))
    int32 MinStartGoalDistance = 3;

    /** The major cycle types that the generator picks from */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Cycles")
    TArray<FNsDungeonCycleWeight> MajorCycles = {
        { ENsDungeonCycleType::LockAndKey, 2 },
        { ENsDungeonCycleType::TwoKeys, 1 },
        { ENsDungeonCycleType::OneWayValve, 1 },
        { ENsDungeonCycleType::Simple, 1 } };

    /** The kind name of a lock and of a key, for the game to show */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Locks")
    FName LockKind = TEXT("Door");

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Locks")
    FName KeyKind = TEXT("Key");

    /** The tries with the next seeds when a seed gives no valid dungeon */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Generation", meta = (ClampMin = "1"))
    int32 MaxSeedTries = 20;
};

/** A rule set for the dungeon generator. The game makes one asset for each kind of floor */
UCLASS(BlueprintType)
class NSDUNGEONS_API UNsDungeonRuleSet : public UPrimaryDataAsset
{
    GENERATED_BODY()

public:
    /** The values of the generation */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "NsDungeons", meta = (ShowOnlyInnerProperties))
    FNsDungeonSettings Settings;
};
