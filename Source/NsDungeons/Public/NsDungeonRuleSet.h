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

/** Where the resolve step may put a room tag */
UENUM(BlueprintType)
enum class ENsDungeonTagPlace : uint8
{
    /** Any room but the start and the goal */
    Any,
    /** A room with one door */
    DeadEnd,
    /** A room on the major cycle */
    MajorCycle,
    /** The room nearest to the goal, then the next ones */
    NearGoal,
    /** The room farthest from the start, then the next ones */
    Far
};

/** A room tag that the resolve step puts on rooms: the game decides what it means, such as a camp or a cache */
USTRUCT(BlueprintType)
struct NSDUNGEONS_API FNsDungeonRoomTag
{
    GENERATED_BODY()

    /** The tag */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "NsDungeons")
    FName Tag;

    /** How many rooms get it */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "NsDungeons", meta = (ClampMin = "0"))
    int32 Count = 1;

    /** Where it goes. With no room of that place left, it goes on any free room */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "NsDungeons")
    ENsDungeonTagPlace Place = ENsDungeonTagPlace::Any;
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

    /** The side loops after the major cycle. Each one adds a short way between two rooms through new rooms */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Cycles", meta = (ClampMin = "0"))
    int32 MinorCycles = 1;

    /** The chance that a minor cycle gets a lock on its way, with the key in a room that the party reaches first */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Cycles", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float MinorLockChance = 0.5f;

    /** The chance of a hidden door that makes a short way from the goal side back to the start side */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Cycles", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float HiddenShortcutChance = 0.5f;

    /** The rooms of a floor. Dead ends grow the floor to this size, as far as the grid allows */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Size", meta = (ClampMin = "2"))
    int32 TargetRooms = 12;

    /** The room tags that the resolve step puts on rooms */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Rooms")
    TArray<FNsDungeonRoomTag> RoomTags;

    /** The smallest and the largest room, in cells on each side. Each room takes a random size between them */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Layout", meta = (ClampMin = "1"))
    int32 RoomCellsMin = 3;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Layout", meta = (ClampMin = "1"))
    int32 RoomCellsMax = 5;

    /** The cells between two blocks of rooms. The corridor between two rooms is at least this long */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Layout", meta = (ClampMin = "1"))
    int32 CorridorCells = 2;

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
