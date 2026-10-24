// Copyright (C) 2025 nulled.softworks. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "NsDungeonGraph.h"
#include "NsDungeonLayout.h"
#include "NsDungeonRuleSet.h"
#include "NsDungeonGenerator.generated.h"

/** The graph after one step of the generation, for the editor preview */
USTRUCT(BlueprintType)
struct NSDUNGEONS_API FNsDungeonStep
{
    GENERATED_BODY()

    /** The name of the step */
    UPROPERTY(BlueprintReadOnly, Category = "NsDungeons")
    FName Name;

    /** What the step did */
    UPROPERTY(BlueprintReadOnly, Category = "NsDungeons")
    FString Note;

    /** The graph after the step */
    UPROPERTY(BlueprintReadOnly, Category = "NsDungeons")
    FNsDungeonGraph Graph;
};

/** The result of a generation */
USTRUCT(BlueprintType)
struct NSDUNGEONS_API FNsDungeonResult
{
    GENERATED_BODY()

    /** The seed that made the dungeon. It can differ from the asked seed when that one failed */
    UPROPERTY(BlueprintReadOnly, Category = "NsDungeons")
    int32 Seed = 0;

    /** True when the goal can be reached and every key can be picked up */
    UPROPERTY(BlueprintReadOnly, Category = "NsDungeons")
    bool bValid = false;

    /** Why the dungeon is not valid, or a short summary */
    UPROPERTY(BlueprintReadOnly, Category = "NsDungeons")
    FString Message;

    /** The dungeon */
    UPROPERTY(BlueprintReadOnly, Category = "NsDungeons")
    FNsDungeonGraph Graph;

    /** The cell layout of the dungeon */
    UPROPERTY(BlueprintReadOnly, Category = "NsDungeons")
    FNsDungeonLayout Layout;

    /** The graph after each step, when the caller asked for them */
    UPROPERTY(BlueprintReadOnly, Category = "NsDungeons")
    TArray<FNsDungeonStep> Steps;
};

/**
 * The dungeon generator, after the method of Unexplored (Boris the Brave, 2021): a fixed grid of nodes, a start and a goal, a major
 * cycle of two arcs from the start to the goal, a cycle type that puts locks, keys, and one-way doors on the arcs, then a check that
 * the goal can be reached. The same settings and seed always give the same dungeon
 */
struct NSDUNGEONS_API FNsDungeonGenerator
{
    /** Makes a dungeon. A seed that gives no valid dungeon tries the next seeds, up to MaxSeedTries. With bKeepSteps, the result keeps the graph after each step */
    static FNsDungeonResult Generate(const FNsDungeonSettings& Settings, int32 Seed, bool bKeepSteps = false);

    /** Makes the cell layout of a graph: a room for each used node, inside its block, and a straight corridor with a door for each edge. The same graph and seed give the same layout */
    static FNsDungeonLayout BuildLayout(const FNsDungeonGraph& Graph, const FNsDungeonSettings& Settings, int32 Seed);

    /** Checks a graph: the party walks from the start, picks up the keys it reaches, and opens the locks it can. Returns true when the goal and every key are reached */
    static bool Validate(const FNsDungeonGraph& Graph, FString& OutMessage);

private:
    /** One try with one seed. Returns false when the steps fail */
    static bool TryGenerate(const FNsDungeonSettings& Settings, FRandomStream& Random, FNsDungeonResult& Result, bool bKeepSteps);

    /** Picks the start and the goal */
    static bool PlaceStartAndGoal(const FNsDungeonSettings& Settings, FRandomStream& Random, FNsDungeonGraph& Graph);

    /** Draws the major cycle: two arcs from the start to the goal that share no other node */
    static bool DrawMajorCycle(FRandomStream& Random, FNsDungeonGraph& Graph, TArray<int32>& OutArc0, TArray<int32>& OutArc1);

    /** Puts the locks, the keys, and the one-way doors of a cycle type on the two arcs */
    static void ApplyCycleType(const FNsDungeonSettings& Settings, ENsDungeonCycleType Type, const TArray<int32>& Arc0, const TArray<int32>& Arc1, FNsDungeonGraph& Graph);

    /** Adds side loops through unused nodes, some with a lock whose key lies nearer to the start */
    static void AddMinorCycles(const FNsDungeonSettings& Settings, FRandomStream& Random, FNsDungeonGraph& Graph);

    /** Adds a hidden door between two rooms that are next to each other on the grid but far apart on the way: a shortcut from the goal side */
    static void AddHiddenShortcut(const FNsDungeonSettings& Settings, FRandomStream& Random, FNsDungeonGraph& Graph);

    /** Grows dead ends from the rooms until the floor has the target number of rooms */
    static void AddDeadEnds(const FNsDungeonSettings& Settings, FRandomStream& Random, FNsDungeonGraph& Graph);

    /** Fills the distances and the dead ends, and puts the room tags of the rule set on rooms */
    static void Resolve(const FNsDungeonSettings& Settings, FRandomStream& Random, FNsDungeonGraph& Graph);

    /** Picks a major cycle type by weight */
    static ENsDungeonCycleType PickCycleType(const FNsDungeonSettings& Settings, FRandomStream& Random);

    /** Finds a path on the grid from one node to another, around the blocked nodes. Random costs bend the path, so each seed gives a new shape */
    static TArray<int32> FindGridPath(const FNsDungeonGraph& Graph, int32 From, int32 To, const TSet<int32>& Blocked, FRandomStream& Random);

    /** Adds a step to the result when the caller keeps them */
    static void KeepStep(FNsDungeonResult& Result, bool bKeepSteps, FName Name, const FString& Note, const FNsDungeonGraph& Graph);
};
