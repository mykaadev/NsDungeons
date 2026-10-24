// Copyright (C) 2025 nulled.softworks. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "NsDungeonGraph.h"
#include "NsDungeonLayout.generated.h"

/** What one cell of the layout is */
UENUM(BlueprintType)
enum class ENsDungeonCell : uint8
{
    /** Solid rock: no floor */
    Empty,
    /** The floor of a room */
    Room,
    /** The floor of a corridor between two rooms */
    Corridor,
    /** A door cell in a corridor. The door of the layout tells its kind */
    Door
};

/** A room of the layout: the node that it comes from and its cells */
USTRUCT(BlueprintType)
struct NSDUNGEONS_API FNsDungeonLayoutRoom
{
    GENERATED_BODY()

    /** The node of the graph */
    UPROPERTY(BlueprintReadOnly, Category = "NsDungeons")
    int32 Node = INDEX_NONE;

    /** The first cell of the room */
    UPROPERTY(BlueprintReadOnly, Category = "NsDungeons")
    FIntPoint Min = FIntPoint::ZeroValue;

    /** The size of the room in cells */
    UPROPERTY(BlueprintReadOnly, Category = "NsDungeons")
    FIntPoint Size = FIntPoint::ZeroValue;

    /** Returns true when a cell lies in the room */
    bool Contains(const FIntPoint& Cell) const { return Cell.X >= Min.X && Cell.Y >= Min.Y && Cell.X < Min.X + Size.X && Cell.Y < Min.Y + Size.Y; }

    /** Returns the middle cell of the room */
    FIntPoint GetCenter() const { return Min + Size / 2; }
};

/** A door of the layout: one cell in the corridor of an edge */
USTRUCT(BlueprintType)
struct NSDUNGEONS_API FNsDungeonLayoutDoor
{
    GENERATED_BODY()

    /** The door cell */
    UPROPERTY(BlueprintReadOnly, Category = "NsDungeons")
    FIntPoint Cell = FIntPoint::ZeroValue;

    /** The edge of the graph */
    UPROPERTY(BlueprintReadOnly, Category = "NsDungeons")
    int32 Edge = INDEX_NONE;

    /** The kind of the edge: open, locked, one-way, or hidden */
    UPROPERTY(BlueprintReadOnly, Category = "NsDungeons")
    ENsDungeonEdgeKind Kind = ENsDungeonEdgeKind::Open;

    /** The direction of the corridor, from node A of the edge to node B: one cell step */
    UPROPERTY(BlueprintReadOnly, Category = "NsDungeons")
    FIntPoint Direction = FIntPoint::ZeroValue;
};

/**
 * The cell layout of a dungeon graph: each used node becomes a room of cells inside its block of the grid, and each edge becomes a
 * straight corridor with one door cell. The game builds its floor and its walk grid from it. Cell (0, 0) is the first cell of the
 * first block. One cell is one step of the walk grid of the game
 */
USTRUCT(BlueprintType)
struct NSDUNGEONS_API FNsDungeonLayout
{
    GENERATED_BODY()

    /** The size of the layout in cells */
    UPROPERTY(BlueprintReadOnly, Category = "NsDungeons")
    FIntPoint Size = FIntPoint::ZeroValue;

    /** Every cell, row by row */
    UPROPERTY(BlueprintReadOnly, Category = "NsDungeons")
    TArray<ENsDungeonCell> Cells;

    /** Every room */
    UPROPERTY(BlueprintReadOnly, Category = "NsDungeons")
    TArray<FNsDungeonLayoutRoom> Rooms;

    /** Every door */
    UPROPERTY(BlueprintReadOnly, Category = "NsDungeons")
    TArray<FNsDungeonLayoutDoor> Doors;

    /** Returns the kind of a cell, or Empty off the layout */
    ENsDungeonCell GetCell(const FIntPoint& Cell) const;

    /** Returns the room of a node, or null */
    const FNsDungeonLayoutRoom* FindRoomOfNode(int32 Node) const;

    /** Returns the room that holds a cell, or null for a corridor or rock */
    const FNsDungeonLayoutRoom* FindRoomAt(const FIntPoint& Cell) const;

    /** Returns the door at a cell, or null */
    const FNsDungeonLayoutDoor* FindDoorAt(const FIntPoint& Cell) const;

    /** Sets a cell. Off the layout it does nothing */
    void SetCell(const FIntPoint& Cell, ENsDungeonCell Kind);
};
