// Copyright (C) 2025 nulled.softworks. All rights reserved.

#include "NsDungeonGraph.h"

void FNsDungeonGraph::Reset(const FIntPoint& InSize)
{
    Size = FIntPoint(FMath::Max(InSize.X, 2), FMath::Max(InSize.Y, 2));
    Nodes.Reset();
    Edges.Reset();
    Locks.Reset();
    Keys.Reset();
    StartNode = INDEX_NONE;
    GoalNode = INDEX_NONE;
    MajorCycle = ENsDungeonCycleType::Simple;
    Nodes.SetNum(Size.X * Size.Y);
    for (int32 Index = 0; Index < Nodes.Num(); ++Index)
    {
        Nodes[Index].Coord = FIntPoint(Index % Size.X, Index / Size.X);
    }
}

bool FNsDungeonGraph::IsInside(const FIntPoint& Coord) const
{
    return Coord.X >= 0 && Coord.Y >= 0 && Coord.X < Size.X && Coord.Y < Size.Y;
}

int32 FNsDungeonGraph::GetIndex(const FIntPoint& Coord) const
{
    return IsInside(Coord) ? Coord.Y * Size.X + Coord.X : INDEX_NONE;
}

TArray<int32> FNsDungeonGraph::GetGridNeighbors(const int32 Node) const
{
    TArray<int32> Neighbors;
    if (!Nodes.IsValidIndex(Node))
    {
        return Neighbors;
    }
    const FIntPoint Coord = Nodes[Node].Coord;
    for (const FIntPoint& Step : { FIntPoint(1, 0), FIntPoint(-1, 0), FIntPoint(0, 1), FIntPoint(0, -1) })
    {
        const int32 Neighbor = GetIndex(Coord + Step);
        if (Neighbor != INDEX_NONE)
        {
            Neighbors.Add(Neighbor);
        }
    }
    return Neighbors;
}

int32 FNsDungeonGraph::FindEdge(const int32 NodeA, const int32 NodeB) const
{
    return Edges.IndexOfByPredicate([NodeA, NodeB](const FNsDungeonEdge& Edge)
    {
        return (Edge.NodeA == NodeA && Edge.NodeB == NodeB) || (Edge.NodeA == NodeB && Edge.NodeB == NodeA);
    });
}

int32 FNsDungeonGraph::Connect(const int32 NodeA, const int32 NodeB, const ENsDungeonEdgeKind Kind)
{
    if (!GetGridNeighbors(NodeA).Contains(NodeB))
    {
        return INDEX_NONE;
    }
    int32 Edge = FindEdge(NodeA, NodeB);
    if (Edge == INDEX_NONE)
    {
        Edge = Edges.AddDefaulted();
        Edges[Edge].NodeA = NodeA;
        Edges[Edge].NodeB = NodeB;
    }
    Edges[Edge].Kind = Kind;
    return Edge;
}

TArray<int32> FNsDungeonGraph::GetEdgesOf(const int32 Node) const
{
    TArray<int32> Found;
    for (int32 Index = 0; Index < Edges.Num(); ++Index)
    {
        if (Edges[Index].NodeA == Node || Edges[Index].NodeB == Node)
        {
            Found.Add(Index);
        }
    }
    return Found;
}

bool FNsDungeonGraph::CanPass(const int32 Edge, const int32 FromNode, const TSet<int32>& HeldKeys) const
{
    if (!Edges.IsValidIndex(Edge))
    {
        return false;
    }
    const FNsDungeonEdge& Link = Edges[Edge];
    switch (Link.Kind)
    {
    case ENsDungeonEdgeKind::Open:
    case ENsDungeonEdgeKind::Hidden:
        return true;
    case ENsDungeonEdgeKind::OneWay:
        return Link.OneWayFrom == FromNode;
    case ENsDungeonEdgeKind::Locked:
    {
        const FNsDungeonLock* const Lock = Locks.FindByPredicate([&Link](const FNsDungeonLock& Item) { return Item.LockId == Link.LockId; });
        // A lock with no key never opens
        if (Lock == nullptr || Lock->KeyIds.IsEmpty())
        {
            return false;
        }
        for (const int32 KeyId : Lock->KeyIds)
        {
            if (!HeldKeys.Contains(KeyId))
            {
                return false;
            }
        }
        return true;
    }
    default:
        return false;
    }
}

int32 FNsDungeonGraph::AddLock(const int32 Edge, const FName Kind)
{
    FNsDungeonLock& Lock = Locks.AddDefaulted_GetRef();
    Lock.LockId = Locks.Num() - 1;
    Lock.Edge = Edge;
    Lock.Kind = Kind;
    Edges[Edge].Kind = ENsDungeonEdgeKind::Locked;
    Edges[Edge].LockId = Lock.LockId;
    return Lock.LockId;
}

int32 FNsDungeonGraph::AddKey(const int32 LockId, const int32 Node, const FName Kind)
{
    FNsDungeonKey& Key = Keys.AddDefaulted_GetRef();
    Key.KeyId = Keys.Num() - 1;
    Key.Node = Node;
    Key.LockId = LockId;
    Key.Kind = Kind;
    if (Locks.IsValidIndex(LockId))
    {
        Locks[LockId].KeyIds.Add(Key.KeyId);
    }
    return Key.KeyId;
}

int32 FNsDungeonGraph::CountUsedNodes() const
{
    return Nodes.FilterByPredicate([](const FNsDungeonNode& Node) { return Node.Role != ENsDungeonNodeRole::Unused; }).Num();
}
