// Copyright (C) 2025 nulled.softworks. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "NsDungeonGraph.generated.h"

/** What a node of the dungeon grid is */
UENUM(BlueprintType)
enum class ENsDungeonNodeRole : uint8
{
    /** Not part of the dungeon */
    Unused,
    /** Where the party comes in */
    Start,
    /** What the party must reach */
    Goal,
    /** A room on the way */
    Path
};

/** How two neighbor nodes connect */
UENUM(BlueprintType)
enum class ENsDungeonEdgeKind : uint8
{
    /** No way through */
    None,
    /** A plain door or opening */
    Open,
    /** A door that needs all the keys of its lock */
    Locked,
    /** A door that opens only from one side: a valve */
    OneWay,
    /** A secret door: open once found */
    Hidden
};

/** The shape of the main loop from the start to the goal (Unexplored: the major cycle) */
UENUM(BlueprintType)
enum class ENsDungeonCycleType : uint8
{
    /** Two routes to the goal, no lock */
    Simple,
    /** The goal sits behind a lock. Its key waits at the end of the other route, which a one-way door closes off from the goal */
    LockAndKey,
    /** As LockAndKey, but the lock needs two keys */
    TwoKeys,
    /** One route leads to the goal. The other is a one-way way back from the goal to the start side */
    OneWayValve
};

/** One node of the dungeon grid. A node becomes a room in the layout */
USTRUCT(BlueprintType)
struct NSDUNGEONS_API FNsDungeonNode
{
    GENERATED_BODY()

    /** What the node is */
    UPROPERTY(BlueprintReadOnly, Category = "NsDungeons")
    ENsDungeonNodeRole Role = ENsDungeonNodeRole::Unused;

    /** The place of the node on the grid */
    UPROPERTY(BlueprintReadOnly, Category = "NsDungeons")
    FIntPoint Coord = FIntPoint::ZeroValue;

    /** The arc of the major cycle that the node is on: 0, 1, or INDEX_NONE */
    UPROPERTY(BlueprintReadOnly, Category = "NsDungeons")
    int32 Arc = INDEX_NONE;

    /** Names that the game gives the node, such as a room type */
    UPROPERTY(BlueprintReadOnly, Category = "NsDungeons")
    TArray<FName> Tags;
};

/** A connection between two neighbor nodes */
USTRUCT(BlueprintType)
struct NSDUNGEONS_API FNsDungeonEdge
{
    GENERATED_BODY()

    /** The two nodes. They are neighbors on the grid */
    UPROPERTY(BlueprintReadOnly, Category = "NsDungeons")
    int32 NodeA = INDEX_NONE;

    UPROPERTY(BlueprintReadOnly, Category = "NsDungeons")
    int32 NodeB = INDEX_NONE;

    /** How the two nodes connect */
    UPROPERTY(BlueprintReadOnly, Category = "NsDungeons")
    ENsDungeonEdgeKind Kind = ENsDungeonEdgeKind::Open;

    /** The lock of a Locked edge, or INDEX_NONE */
    UPROPERTY(BlueprintReadOnly, Category = "NsDungeons")
    int32 LockId = INDEX_NONE;

    /** The node that a OneWay edge can be passed from, or INDEX_NONE */
    UPROPERTY(BlueprintReadOnly, Category = "NsDungeons")
    int32 OneWayFrom = INDEX_NONE;

    /** Returns the other node of the edge */
    int32 GetOther(int32 Node) const { return Node == NodeA ? NodeB : NodeA; }
};

/** A lock: a Locked edge and the keys that open it */
USTRUCT(BlueprintType)
struct NSDUNGEONS_API FNsDungeonLock
{
    GENERATED_BODY()

    /** The id of the lock */
    UPROPERTY(BlueprintReadOnly, Category = "NsDungeons")
    int32 LockId = INDEX_NONE;

    /** The edge that the lock closes */
    UPROPERTY(BlueprintReadOnly, Category = "NsDungeons")
    int32 Edge = INDEX_NONE;

    /** Every key that the lock needs */
    UPROPERTY(BlueprintReadOnly, Category = "NsDungeons")
    TArray<int32> KeyIds;

    /** The kind of lock that the game shows, from the rule set */
    UPROPERTY(BlueprintReadOnly, Category = "NsDungeons")
    FName Kind;
};

/** A key: a node that holds it, and the lock that it opens */
USTRUCT(BlueprintType)
struct NSDUNGEONS_API FNsDungeonKey
{
    GENERATED_BODY()

    /** The id of the key */
    UPROPERTY(BlueprintReadOnly, Category = "NsDungeons")
    int32 KeyId = INDEX_NONE;

    /** The node that holds the key */
    UPROPERTY(BlueprintReadOnly, Category = "NsDungeons")
    int32 Node = INDEX_NONE;

    /** The lock that the key opens */
    UPROPERTY(BlueprintReadOnly, Category = "NsDungeons")
    int32 LockId = INDEX_NONE;

    /** The kind of key that the game shows, from the rule set */
    UPROPERTY(BlueprintReadOnly, Category = "NsDungeons")
    FName Kind;
};

/**
 * The abstract dungeon: a fixed grid of nodes, the edges between neighbors, and the locks and keys. Nodes never move, as in Unexplored:
 * the steps only mark them, so the grid stays a grid and the layout follows it directly
 */
USTRUCT(BlueprintType)
struct NSDUNGEONS_API FNsDungeonGraph
{
    GENERATED_BODY()

    /** The size of the grid */
    UPROPERTY(BlueprintReadOnly, Category = "NsDungeons")
    FIntPoint Size = FIntPoint(5, 5);

    /** Every node, row by row */
    UPROPERTY(BlueprintReadOnly, Category = "NsDungeons")
    TArray<FNsDungeonNode> Nodes;

    /** Every connection */
    UPROPERTY(BlueprintReadOnly, Category = "NsDungeons")
    TArray<FNsDungeonEdge> Edges;

    /** Every lock */
    UPROPERTY(BlueprintReadOnly, Category = "NsDungeons")
    TArray<FNsDungeonLock> Locks;

    /** Every key */
    UPROPERTY(BlueprintReadOnly, Category = "NsDungeons")
    TArray<FNsDungeonKey> Keys;

    /** The start and the goal node */
    UPROPERTY(BlueprintReadOnly, Category = "NsDungeons")
    int32 StartNode = INDEX_NONE;

    UPROPERTY(BlueprintReadOnly, Category = "NsDungeons")
    int32 GoalNode = INDEX_NONE;

    /** The shape of the main loop */
    UPROPERTY(BlueprintReadOnly, Category = "NsDungeons")
    ENsDungeonCycleType MajorCycle = ENsDungeonCycleType::Simple;

    /** Makes an empty grid of unused nodes */
    void Reset(const FIntPoint& InSize);

    /** Returns true when a coordinate lies on the grid */
    bool IsInside(const FIntPoint& Coord) const;

    /** Returns the index of a node from its coordinate, or INDEX_NONE */
    int32 GetIndex(const FIntPoint& Coord) const;

    /** Returns the up to 4 grid neighbors of a node */
    TArray<int32> GetGridNeighbors(int32 Node) const;

    /** Returns the edge between two nodes, or INDEX_NONE */
    int32 FindEdge(int32 NodeA, int32 NodeB) const;

    /** Adds an edge between two grid neighbors, or changes the kind of the edge that exists. Returns its index */
    int32 Connect(int32 NodeA, int32 NodeB, ENsDungeonEdgeKind Kind);

    /** Returns every edge of a node */
    TArray<int32> GetEdgesOf(int32 Node) const;

    /** Returns true when the party can pass an edge from a node, with a set of keys */
    bool CanPass(int32 Edge, int32 FromNode, const TSet<int32>& HeldKeys) const;

    /** Adds a lock on an edge and returns its id */
    int32 AddLock(int32 Edge, FName Kind);

    /** Adds a key for a lock on a node and returns its id */
    int32 AddKey(int32 LockId, int32 Node, FName Kind);

    /** Returns the number of nodes that are part of the dungeon */
    int32 CountUsedNodes() const;
};
