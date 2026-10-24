// Copyright (C) 2025 nulled.softworks. All rights reserved.

#include "NsDungeonGenerator.h"

#include "NsDungeons.h"

FNsDungeonResult FNsDungeonGenerator::Generate(const FNsDungeonSettings& Settings, const int32 Seed, const bool bKeepSteps)
{
    FNsDungeonResult Result;
    for (int32 Try = 0; Try < FMath::Max(Settings.MaxSeedTries, 1); ++Try)
    {
        Result = FNsDungeonResult();
        Result.Seed = Seed + Try;
        FRandomStream Random(Result.Seed);
        if (TryGenerate(Settings, Random, Result, bKeepSteps) && Validate(Result.Graph, Result.Message))
        {
            Result.bValid = true;
            Result.Layout = BuildLayout(Result.Graph, Settings, Result.Seed);
            if (Try > 0)
            {
                UE_LOG(LogNsDungeons, Log, TEXT("Seed %d gave no valid dungeon. Seed %d did."), Seed, Result.Seed);
            }
            return Result;
        }
    }
    UE_LOG(LogNsDungeons, Error, TEXT("No valid dungeon from seed %d in %d tries: %s. Check the grid size and the start to goal distance of the rule set."),
        Seed, Settings.MaxSeedTries, *Result.Message);
    return Result;
}

bool FNsDungeonGenerator::TryGenerate(const FNsDungeonSettings& Settings, FRandomStream& Random, FNsDungeonResult& Result, const bool bKeepSteps)
{
    FNsDungeonGraph& Graph = Result.Graph;
    Graph.Reset(Settings.GridSize);
    KeepStep(Result, bKeepSteps, TEXT("Grid"), FString::Printf(TEXT("A grid of %d by %d nodes"), Graph.Size.X, Graph.Size.Y), Graph);

    if (!PlaceStartAndGoal(Settings, Random, Graph))
    {
        Result.Message = TEXT("No two nodes far enough apart for the start and the goal");
        return false;
    }
    KeepStep(Result, bKeepSteps, TEXT("StartAndGoal"), TEXT("The start and the goal"), Graph);

    TArray<int32> Arc0;
    TArray<int32> Arc1;
    if (!DrawMajorCycle(Random, Graph, Arc0, Arc1))
    {
        Result.Message = TEXT("No loop of two arcs from the start to the goal");
        return false;
    }
    KeepStep(Result, bKeepSteps, TEXT("MajorCycle"), FString::Printf(TEXT("A loop of two arcs: %d and %d nodes"), Arc0.Num(), Arc1.Num()), Graph);

    Graph.MajorCycle = PickCycleType(Settings, Random);
    ApplyCycleType(Settings, Graph.MajorCycle, Arc0, Arc1, Graph);
    KeepStep(Result, bKeepSteps, TEXT("CycleType"), FString::Printf(TEXT("Cycle type %s"), *UEnum::GetValueAsString(Graph.MajorCycle)), Graph);

    AddMinorCycles(Settings, Random, Graph);
    KeepStep(Result, bKeepSteps, TEXT("MinorCycles"), FString::Printf(TEXT("Side loops, %d locks in all"), Graph.Locks.Num()), Graph);

    AddHiddenShortcut(Settings, Random, Graph);
    KeepStep(Result, bKeepSteps, TEXT("HiddenShortcut"), TEXT("A hidden door, by chance"), Graph);

    AddDeadEnds(Settings, Random, Graph);
    KeepStep(Result, bKeepSteps, TEXT("DeadEnds"), FString::Printf(TEXT("%d rooms"), Graph.CountUsedNodes()), Graph);

    Resolve(Settings, Random, Graph);
    KeepStep(Result, bKeepSteps, TEXT("Resolve"), TEXT("Distances, dead ends, and room tags"), Graph);
    return true;
}

bool FNsDungeonGenerator::PlaceStartAndGoal(const FNsDungeonSettings& Settings, FRandomStream& Random, FNsDungeonGraph& Graph)
{
    const int32 Start = Random.RandRange(0, Graph.Nodes.Num() - 1);
    TArray<int32> Far;
    for (int32 Index = 0; Index < Graph.Nodes.Num(); ++Index)
    {
        const FIntPoint Gap = Graph.Nodes[Index].Coord - Graph.Nodes[Start].Coord;
        if (FMath::Abs(Gap.X) + FMath::Abs(Gap.Y) >= Settings.MinStartGoalDistance)
        {
            Far.Add(Index);
        }
    }
    if (Far.IsEmpty())
    {
        return false;
    }
    Graph.StartNode = Start;
    Graph.GoalNode = Far[Random.RandRange(0, Far.Num() - 1)];
    Graph.Nodes[Graph.StartNode].Role = ENsDungeonNodeRole::Start;
    Graph.Nodes[Graph.GoalNode].Role = ENsDungeonNodeRole::Goal;
    return true;
}

bool FNsDungeonGenerator::DrawMajorCycle(FRandomStream& Random, FNsDungeonGraph& Graph, TArray<int32>& OutArc0, TArray<int32>& OutArc1)
{
    // A few tries with new random costs: the first arc can cut the grid so that no second arc fits
    for (int32 Try = 0; Try < 8; ++Try)
    {
        OutArc0 = FindGridPath(Graph, Graph.StartNode, Graph.GoalNode, TSet<int32>(), Random);
        if (OutArc0.Num() < 3)
        {
            continue;
        }
        TSet<int32> Blocked(OutArc0);
        Blocked.Remove(Graph.StartNode);
        Blocked.Remove(Graph.GoalNode);
        OutArc1 = FindGridPath(Graph, Graph.StartNode, Graph.GoalNode, Blocked, Random);
        if (OutArc1.Num() < 3)
        {
            continue;
        }
        for (int32 ArcIndex = 0; ArcIndex < 2; ++ArcIndex)
        {
            const TArray<int32>& Arc = ArcIndex == 0 ? OutArc0 : OutArc1;
            for (int32 Step = 0; Step < Arc.Num(); ++Step)
            {
                FNsDungeonNode& Node = Graph.Nodes[Arc[Step]];
                if (Node.Role == ENsDungeonNodeRole::Unused)
                {
                    Node.Role = ENsDungeonNodeRole::Path;
                    Node.Arc = ArcIndex;
                }
                if (Step > 0)
                {
                    Graph.Connect(Arc[Step - 1], Arc[Step], ENsDungeonEdgeKind::Open);
                }
            }
        }
        return true;
    }
    return false;
}

void FNsDungeonGenerator::ApplyCycleType(const FNsDungeonSettings& Settings, const ENsDungeonCycleType Type, const TArray<int32>& Arc0, const TArray<int32>& Arc1,
    FNsDungeonGraph& Graph)
{
    // The last node of each arc before the goal, and the edges from there into the goal
    const int32 Before0 = Arc0[Arc0.Num() - 2];
    const int32 Before1 = Arc1[Arc1.Num() - 2];
    const int32 Into0 = Graph.FindEdge(Before0, Graph.GoalNode);
    const int32 Into1 = Graph.FindEdge(Before1, Graph.GoalNode);
    const auto MakeValve = [&Graph, Into1]()
    {
        // Arc 1 opens only from the goal side: a short way back, and a dead end on the way out
        Graph.Edges[Into1].Kind = ENsDungeonEdgeKind::OneWay;
        Graph.Edges[Into1].OneWayFrom = Graph.GoalNode;
    };
    switch (Type)
    {
    case ENsDungeonCycleType::LockAndKey:
    {
        const int32 Lock = Graph.AddLock(Into0, Settings.LockKind);
        Graph.AddKey(Lock, Before1, Settings.KeyKind);
        MakeValve();
        break;
    }
    case ENsDungeonCycleType::TwoKeys:
    {
        // One key waits at the end of arc 1, the other halfway along arc 0
        const int32 Lock = Graph.AddLock(Into0, Settings.LockKind);
        Graph.AddKey(Lock, Before1, Settings.KeyKind);
        Graph.AddKey(Lock, Arc0[(Arc0.Num() - 1) / 2], Settings.KeyKind);
        MakeValve();
        break;
    }
    case ENsDungeonCycleType::OneWayValve:
        MakeValve();
        break;
    default:
        break;
    }
}

ENsDungeonCycleType FNsDungeonGenerator::PickCycleType(const FNsDungeonSettings& Settings, FRandomStream& Random)
{
    int32 Total = 0;
    for (const FNsDungeonCycleWeight& Entry : Settings.MajorCycles)
    {
        Total += FMath::Max(Entry.Weight, 0);
    }
    if (Total <= 0)
    {
        return ENsDungeonCycleType::Simple;
    }
    int32 Roll = Random.RandRange(0, Total - 1);
    for (const FNsDungeonCycleWeight& Entry : Settings.MajorCycles)
    {
        Roll -= FMath::Max(Entry.Weight, 0);
        if (Roll < 0)
        {
            return Entry.Type;
        }
    }
    return ENsDungeonCycleType::Simple;
}

TArray<int32> FNsDungeonGenerator::FindGridPath(const FNsDungeonGraph& Graph, const int32 From, const int32 To, const TSet<int32>& Blocked, FRandomStream& Random)
{
    // Dijkstra with a random cost on each node: the shortest path under the costs wanders, but it stays a simple path
    TArray<int32> Cost;
    Cost.SetNum(Graph.Nodes.Num());
    for (int32& Value : Cost)
    {
        Value = Random.RandRange(1, 6);
    }
    TArray<int32> Distance;
    TArray<int32> Previous;
    Distance.Init(MAX_int32, Graph.Nodes.Num());
    Previous.Init(INDEX_NONE, Graph.Nodes.Num());
    TArray<bool> Done;
    Done.Init(false, Graph.Nodes.Num());
    Distance[From] = 0;
    for (int32 Round = 0; Round < Graph.Nodes.Num(); ++Round)
    {
        int32 Best = INDEX_NONE;
        for (int32 Index = 0; Index < Graph.Nodes.Num(); ++Index)
        {
            if (!Done[Index] && Distance[Index] != MAX_int32 && (Best == INDEX_NONE || Distance[Index] < Distance[Best]))
            {
                Best = Index;
            }
        }
        if (Best == INDEX_NONE || Best == To)
        {
            break;
        }
        Done[Best] = true;
        for (const int32 Next : Graph.GetGridNeighbors(Best))
        {
            if (Done[Next] || (Next != To && Blocked.Contains(Next)) || (Best == From && Next == To))
            {
                // A direct step from the start to the goal makes no loop: each arc needs a node between them
                continue;
            }
            if (Distance[Best] + Cost[Next] < Distance[Next])
            {
                Distance[Next] = Distance[Best] + Cost[Next];
                Previous[Next] = Best;
            }
        }
    }
    TArray<int32> Path;
    if (Distance[To] == MAX_int32)
    {
        return Path;
    }
    for (int32 Node = To; Node != INDEX_NONE; Node = Previous[Node])
    {
        Path.Insert(Node, 0);
    }
    return Path;
}

bool FNsDungeonGenerator::Validate(const FNsDungeonGraph& Graph, FString& OutMessage)
{
    if (!Graph.Nodes.IsValidIndex(Graph.StartNode) || !Graph.Nodes.IsValidIndex(Graph.GoalNode))
    {
        OutMessage = TEXT("The graph has no start or no goal");
        return false;
    }
    // The party walks from the start, picks up each key it reaches, and walks again with the new keys until nothing changes
    TSet<int32> HeldKeys;
    TSet<int32> Reached;
    for (bool bChanged = true; bChanged;)
    {
        bChanged = false;
        Reached.Reset();
        TArray<int32> Open = { Graph.StartNode };
        Reached.Add(Graph.StartNode);
        while (!Open.IsEmpty())
        {
            const int32 Node = Open.Pop();
            for (const int32 Edge : Graph.GetEdgesOf(Node))
            {
                const int32 Next = Graph.Edges[Edge].GetOther(Node);
                if (!Reached.Contains(Next) && Graph.CanPass(Edge, Node, HeldKeys))
                {
                    Reached.Add(Next);
                    Open.Add(Next);
                }
            }
        }
        for (const FNsDungeonKey& Key : Graph.Keys)
        {
            if (Reached.Contains(Key.Node) && !HeldKeys.Contains(Key.KeyId))
            {
                HeldKeys.Add(Key.KeyId);
                bChanged = true;
            }
        }
    }
    if (!Reached.Contains(Graph.GoalNode))
    {
        OutMessage = TEXT("The goal cannot be reached");
        return false;
    }
    if (HeldKeys.Num() != Graph.Keys.Num())
    {
        OutMessage = FString::Printf(TEXT("%d of %d keys cannot be reached"), Graph.Keys.Num() - HeldKeys.Num(), Graph.Keys.Num());
        return false;
    }
    OutMessage = FString::Printf(TEXT("Valid: %d rooms, cycle %s, %d locks, %d keys"), Graph.CountUsedNodes(), *UEnum::GetValueAsString(Graph.MajorCycle),
        Graph.Locks.Num(), Graph.Keys.Num());
    return true;
}

void FNsDungeonGenerator::KeepStep(FNsDungeonResult& Result, const bool bKeepSteps, const FName Name, const FString& Note, const FNsDungeonGraph& Graph)
{
    if (bKeepSteps)
    {
        FNsDungeonStep& Step = Result.Steps.AddDefaulted_GetRef();
        Step.Name = Name;
        Step.Note = Note;
        Step.Graph = Graph;
    }
}
