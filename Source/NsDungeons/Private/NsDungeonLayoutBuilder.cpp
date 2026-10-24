// Copyright (C) 2025 nulled.softworks. All rights reserved.

#include "NsDungeonGenerator.h"

// The layout step (D3): the graph becomes cells. Each node owns a square block of RoomCellsMax cells, with CorridorCells of rock between
// blocks. A room takes a random size and sits in the middle of its block, so the middle row and the middle column of every block cross
// every room: the corridor of an edge runs straight along them, and its middle cell is the door

FNsDungeonLayout FNsDungeonGenerator::BuildLayout(const FNsDungeonGraph& Graph, const FNsDungeonSettings& Settings, const int32 Seed)
{
    FNsDungeonLayout Layout;
    const int32 Block = FMath::Max3(Settings.RoomCellsMax, Settings.RoomCellsMin, 1);
    const int32 Smallest = FMath::Clamp(Settings.RoomCellsMin, 1, Block);
    const int32 Gap = FMath::Max(Settings.CorridorCells, 1);
    const int32 Stride = Block + Gap;
    Layout.Size = FIntPoint(Graph.Size.X * Stride - Gap, Graph.Size.Y * Stride - Gap);
    Layout.Cells.Init(ENsDungeonCell::Empty, Layout.Size.X * Layout.Size.Y);

    // Its own stream: the layout of a graph stays the same when the graph steps change their draws
    FRandomStream Random(HashCombine(GetTypeHash(Seed), GetTypeHash(TEXT("NsDungeonsLayout"))));
    for (int32 Node = 0; Node < Graph.Nodes.Num(); ++Node)
    {
        if (!Graph.IsUsed(Node))
        {
            continue;
        }
        FNsDungeonLayoutRoom& Room = Layout.Rooms.AddDefaulted_GetRef();
        Room.Node = Node;
        Room.Size = FIntPoint(Random.RandRange(Smallest, Block), Random.RandRange(Smallest, Block));
        Room.Min = Graph.Nodes[Node].Coord * Stride + (FIntPoint(Block, Block) - Room.Size) / 2;
        for (int32 Y = 0; Y < Room.Size.Y; ++Y)
        {
            for (int32 X = 0; X < Room.Size.X; ++X)
            {
                Layout.SetCell(Room.Min + FIntPoint(X, Y), ENsDungeonCell::Room);
            }
        }
    }

    for (int32 Edge = 0; Edge < Graph.Edges.Num(); ++Edge)
    {
        const FNsDungeonEdge& Link = Graph.Edges[Edge];
        const FNsDungeonLayoutRoom* const RoomA = Layout.FindRoomOfNode(Link.NodeA);
        const FNsDungeonLayoutRoom* const RoomB = Layout.FindRoomOfNode(Link.NodeB);
        if (RoomA == nullptr || RoomB == nullptr)
        {
            continue;
        }
        // From the cell next to room A to the cell next to room B, along the middle line of the two blocks
        const FIntPoint Step = Graph.Nodes[Link.NodeB].Coord - Graph.Nodes[Link.NodeA].Coord;
        const FIntPoint Middle = Graph.Nodes[Link.NodeA].Coord * Stride + FIntPoint(Block / 2, Block / 2);
        FIntPoint Cell = Middle;
        while (RoomA->Contains(Cell))
        {
            Cell += Step;
        }
        TArray<FIntPoint> Corridor;
        while (!RoomB->Contains(Cell) && Corridor.Num() < Stride * 2)
        {
            Corridor.Add(Cell);
            Cell += Step;
        }
        if (Corridor.IsEmpty())
        {
            continue;
        }
        for (const FIntPoint& Way : Corridor)
        {
            Layout.SetCell(Way, ENsDungeonCell::Corridor);
        }
        FNsDungeonLayoutDoor& Door = Layout.Doors.AddDefaulted_GetRef();
        Door.Cell = Corridor[Corridor.Num() / 2];
        Door.Edge = Edge;
        Door.Kind = Link.Kind;
        Door.Direction = Step;
        Layout.SetCell(Door.Cell, ENsDungeonCell::Door);
    }
    return Layout;
}
