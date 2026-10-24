// Copyright (C) 2025 nulled.softworks. All rights reserved.

#include "NsDungeonGenerator.h"

// The steps after the major cycle (D2): minor cycles, the hidden shortcut, the dead ends, and the resolve step

void FNsDungeonGenerator::AddMinorCycles(const FNsDungeonSettings& Settings, FRandomStream& Random, FNsDungeonGraph& Graph)
{
    for (int32 Cycle = 0; Cycle < Settings.MinorCycles; ++Cycle)
    {
        for (int32 Try = 0; Try < 20; ++Try)
        {
            // From a room, out through unused nodes, and back into another room: a side loop
            TArray<int32> Used;
            for (int32 Node = 0; Node < Graph.Nodes.Num(); ++Node)
            {
                if (Graph.IsUsed(Node) && Node != Graph.GoalNode)
                {
                    Used.Add(Node);
                }
            }
            const int32 From = Used[Random.RandRange(0, Used.Num() - 1)];
            TArray<int32> Free = Graph.GetGridNeighbors(From).FilterByPredicate([&Graph](const int32 Node) { return !Graph.IsUsed(Node); });
            if (Free.IsEmpty())
            {
                continue;
            }
            // Walk 1 to 3 unused nodes, and stop next to a room that is not the start of the loop
            TArray<int32> Way = { Free[Random.RandRange(0, Free.Num() - 1)] };
            int32 Into = INDEX_NONE;
            for (int32 Step = 0; Step < 3 && Into == INDEX_NONE; ++Step)
            {
                TArray<int32> Rooms = Graph.GetGridNeighbors(Way.Last()).FilterByPredicate([&Graph, From](const int32 Node)
                {
                    return Graph.IsUsed(Node) && Node != From && Node != Graph.GoalNode;
                });
                if (!Rooms.IsEmpty() && Step > 0)
                {
                    Into = Rooms[Random.RandRange(0, Rooms.Num() - 1)];
                    break;
                }
                TArray<int32> Next = Graph.GetGridNeighbors(Way.Last()).FilterByPredicate([&Graph, &Way](const int32 Node)
                {
                    return !Graph.IsUsed(Node) && !Way.Contains(Node);
                });
                if (Next.IsEmpty())
                {
                    if (!Rooms.IsEmpty())
                    {
                        Into = Rooms[Random.RandRange(0, Rooms.Num() - 1)];
                    }
                    break;
                }
                Way.Add(Next[Random.RandRange(0, Next.Num() - 1)]);
            }
            if (Into == INDEX_NONE)
            {
                continue;
            }
            for (const int32 Node : Way)
            {
                Graph.Nodes[Node].Role = ENsDungeonNodeRole::Path;
                Graph.Nodes[Node].bMinorCycle = true;
            }
            const int32 Entry = Graph.Connect(From, Way[0], ENsDungeonEdgeKind::Open);
            for (int32 Step = 1; Step < Way.Num(); ++Step)
            {
                Graph.Connect(Way[Step - 1], Way[Step], ENsDungeonEdgeKind::Open);
            }
            const int32 Exit = Graph.Connect(Way.Last(), Into, ENsDungeonEdgeKind::Open);

            // A locked side loop: a lock at the entry, a one-way door out at the exit, and the key in a room that the party reaches first.
            // The check walks the dungeon: a key that cannot be reached takes the lock back off
            if (Random.FRand() < Settings.MinorLockChance)
            {
                const int32 Lock = Graph.AddLock(Entry, Settings.LockKind);
                Graph.Edges[Exit].Kind = ENsDungeonEdgeKind::OneWay;
                Graph.Edges[Exit].OneWayFrom = Way.Last();
                TArray<int32> KeyRooms = Used.FilterByPredicate([&Graph](const int32 Node) { return Node != Graph.StartNode; });
                Graph.AddKey(Lock, KeyRooms.IsEmpty() ? Graph.StartNode : KeyRooms[Random.RandRange(0, KeyRooms.Num() - 1)], Settings.KeyKind);
                FString Message;
                if (!Validate(Graph, Message))
                {
                    Graph.Keys.Pop();
                    Graph.Locks.Pop();
                    Graph.Edges[Entry].Kind = ENsDungeonEdgeKind::Open;
                    Graph.Edges[Entry].LockId = INDEX_NONE;
                    Graph.Edges[Exit].Kind = ENsDungeonEdgeKind::Open;
                    Graph.Edges[Exit].OneWayFrom = INDEX_NONE;
                }
            }
            break;
        }
    }
}

void FNsDungeonGenerator::AddHiddenShortcut(const FNsDungeonSettings& Settings, FRandomStream& Random, FNsDungeonGraph& Graph)
{
    if (Random.FRand() >= Settings.HiddenShortcutChance)
    {
        return;
    }
    // Two rooms side by side on the grid that are far apart on the way: the best pair saves the most doors
    const TArray<int32> Distances = Graph.GetDistances(Graph.StartNode);
    int32 BestA = INDEX_NONE;
    int32 BestB = INDEX_NONE;
    int32 BestGain = 2;
    for (int32 Node = 0; Node < Graph.Nodes.Num(); ++Node)
    {
        for (const int32 Next : Graph.GetGridNeighbors(Node))
        {
            if (Node < Next && Graph.IsUsed(Node) && Graph.IsUsed(Next) && Graph.FindEdge(Node, Next) == INDEX_NONE
                && Distances[Node] != INDEX_NONE && Distances[Next] != INDEX_NONE && FMath::Abs(Distances[Node] - Distances[Next]) > BestGain)
            {
                BestGain = FMath::Abs(Distances[Node] - Distances[Next]);
                BestA = Node;
                BestB = Next;
            }
        }
    }
    if (BestA != INDEX_NONE)
    {
        Graph.Connect(BestA, BestB, ENsDungeonEdgeKind::Hidden);
    }
}

void FNsDungeonGenerator::AddDeadEnds(const FNsDungeonSettings& Settings, FRandomStream& Random, FNsDungeonGraph& Graph)
{
    while (Graph.CountUsedNodes() < Settings.TargetRooms)
    {
        // A room that is not the goal, next to an unused node
        TArray<TPair<int32, int32>> Grows;
        for (int32 Node = 0; Node < Graph.Nodes.Num(); ++Node)
        {
            if (!Graph.IsUsed(Node) || Node == Graph.GoalNode)
            {
                continue;
            }
            for (const int32 Next : Graph.GetGridNeighbors(Node))
            {
                if (!Graph.IsUsed(Next))
                {
                    Grows.Add({ Node, Next });
                }
            }
        }
        if (Grows.IsEmpty())
        {
            return;
        }
        const TPair<int32, int32>& Grow = Grows[Random.RandRange(0, Grows.Num() - 1)];
        Graph.Nodes[Grow.Value].Role = ENsDungeonNodeRole::Path;
        Graph.Connect(Grow.Key, Grow.Value, ENsDungeonEdgeKind::Open);
    }
}

void FNsDungeonGenerator::Resolve(const FNsDungeonSettings& Settings, FRandomStream& Random, FNsDungeonGraph& Graph)
{
    const TArray<int32> FromStart = Graph.GetDistances(Graph.StartNode);
    const TArray<int32> FromGoal = Graph.GetDistances(Graph.GoalNode);
    TArray<int32> Rooms;
    for (int32 Node = 0; Node < Graph.Nodes.Num(); ++Node)
    {
        FNsDungeonNode& Room = Graph.Nodes[Node];
        Room.Distance = FromStart[Node];
        Room.bDeadEnd = Room.Role == ENsDungeonNodeRole::Path && Graph.GetEdgesOf(Node).Num() == 1;
        if (Room.Role == ENsDungeonNodeRole::Path)
        {
            Rooms.Add(Node);
        }
    }

    // Each tag goes on free rooms of its place, one tag for each room. With none of that place left, any free room takes it
    for (const FNsDungeonRoomTag& Rule : Settings.RoomTags)
    {
        for (int32 Count = 0; Count < Rule.Count; ++Count)
        {
            TArray<int32> Free = Rooms.FilterByPredicate([&Graph](const int32 Node) { return Graph.Nodes[Node].Tags.IsEmpty(); });
            if (Free.IsEmpty())
            {
                break;
            }
            TArray<int32> Fit = Free.FilterByPredicate([&Graph, &Rule](const int32 Node)
            {
                const FNsDungeonNode& Room = Graph.Nodes[Node];
                switch (Rule.Place)
                {
                case ENsDungeonTagPlace::DeadEnd: return Room.bDeadEnd;
                case ENsDungeonTagPlace::MajorCycle: return Room.Arc != INDEX_NONE;
                default: return true;
                }
            });
            TArray<int32>& Pool = Fit.IsEmpty() ? Free : Fit;
            int32 Pick = Pool[Random.RandRange(0, Pool.Num() - 1)];
            if (Rule.Place == ENsDungeonTagPlace::NearGoal || Rule.Place == ENsDungeonTagPlace::Far)
            {
                // The nearest room to the goal, or the farthest from the start
                Pool.Sort([&FromStart, &FromGoal, &Rule](const int32 A, const int32 B)
                {
                    return Rule.Place == ENsDungeonTagPlace::NearGoal ? FromGoal[A] < FromGoal[B] : FromStart[A] > FromStart[B];
                });
                Pick = Pool[0];
            }
            Graph.Nodes[Pick].Tags.Add(Rule.Tag);
        }
    }
}
