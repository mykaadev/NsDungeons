// Copyright (C) 2025 nulled.softworks. All rights reserved.

#include "Misc/AutomationTest.h"
#include "NsDungeonGenerator.h"

#if WITH_DEV_AUTOMATION_TESTS

namespace NsDungeonTests
{
    /** Settings that pick only one major cycle type */
    FNsDungeonSettings MakeSettings(const ENsDungeonCycleType Type)
    {
        FNsDungeonSettings Settings;
        Settings.MajorCycles = { { Type, 1 } };
        return Settings;
    }
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FNsDungeonCycleTypesTest, "NsDungeons.Generator.CycleTypes", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FNsDungeonCycleTypesTest::RunTest(const FString& Parameters)
{
    // Each cycle type gives a valid dungeon for 200 seeds, with the locks and keys of its type
    for (const ENsDungeonCycleType Type : { ENsDungeonCycleType::Simple, ENsDungeonCycleType::LockAndKey, ENsDungeonCycleType::TwoKeys, ENsDungeonCycleType::OneWayValve })
    {
        const FNsDungeonSettings Settings = NsDungeonTests::MakeSettings(Type);
        const int32 ExpectedKeys = Type == ENsDungeonCycleType::TwoKeys ? 2 : Type == ENsDungeonCycleType::LockAndKey ? 1 : 0;
        int32 Failures = 0;
        for (int32 Seed = 0; Seed < 200; ++Seed)
        {
            const FNsDungeonResult Result = FNsDungeonGenerator::Generate(Settings, Seed * 7919);
            const FNsDungeonGraph& Graph = Result.Graph;
            const bool bShape = Result.bValid && Graph.MajorCycle == Type && Graph.StartNode != Graph.GoalNode && Graph.CountUsedNodes() >= 4
                && Graph.Keys.Num() == ExpectedKeys && Graph.Locks.Num() == (ExpectedKeys > 0 ? 1 : 0);
            Failures += bShape ? 0 : 1;
        }
        TestEqual(FString::Printf(TEXT("%s: every seed gives a valid dungeon of its shape"), *UEnum::GetValueAsString(Type)), Failures, 0);
    }
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FNsDungeonLockBlocksTest, "NsDungeons.Generator.LockBlocksGoal", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FNsDungeonLockBlocksTest::RunTest(const FString& Parameters)
{
    // Without its keys, a lock and key dungeon has no way to the goal: the lock matters
    for (int32 Seed = 0; Seed < 50; ++Seed)
    {
        FNsDungeonResult Result = FNsDungeonGenerator::Generate(NsDungeonTests::MakeSettings(ENsDungeonCycleType::LockAndKey), Seed);
        Result.Graph.Keys.Reset();
        for (FNsDungeonLock& Lock : Result.Graph.Locks)
        {
            Lock.KeyIds = { 99 };
        }
        FString Message;
        if (!TestFalse(FString::Printf(TEXT("Seed %d: the goal needs the key"), Seed), FNsDungeonGenerator::Validate(Result.Graph, Message)))
        {
            break;
        }
    }
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FNsDungeonSameSeedTest, "NsDungeons.Generator.SameSeed", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FNsDungeonSameSeedTest::RunTest(const FString& Parameters)
{
    // The same settings and seed give the same dungeon, and the steps end on the result
    const FNsDungeonSettings Settings;
    const FNsDungeonResult First = FNsDungeonGenerator::Generate(Settings, 1234, true);
    const FNsDungeonResult Second = FNsDungeonGenerator::Generate(Settings, 1234);
    TestEqual(TEXT("Same seed"), First.Seed, Second.Seed);
    TestEqual(TEXT("Same start"), First.Graph.StartNode, Second.Graph.StartNode);
    TestEqual(TEXT("Same goal"), First.Graph.GoalNode, Second.Graph.GoalNode);
    TestEqual(TEXT("Same number of edges"), First.Graph.Edges.Num(), Second.Graph.Edges.Num());
    bool bSameEdges = First.Graph.Edges.Num() == Second.Graph.Edges.Num();
    for (int32 Index = 0; bSameEdges && Index < First.Graph.Edges.Num(); ++Index)
    {
        bSameEdges = First.Graph.Edges[Index].NodeA == Second.Graph.Edges[Index].NodeA && First.Graph.Edges[Index].NodeB == Second.Graph.Edges[Index].NodeB
            && First.Graph.Edges[Index].Kind == Second.Graph.Edges[Index].Kind;
    }
    TestTrue(TEXT("Same edges"), bSameEdges);
    TestTrue(TEXT("The steps are kept"), First.Steps.Num() >= 4);
    TestTrue(TEXT("The last step is the result"), !First.Steps.IsEmpty() && First.Steps.Last().Graph.Edges.Num() == First.Graph.Edges.Num());
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FNsDungeonValidateTest, "NsDungeons.Generator.Validate", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FNsDungeonValidateTest::RunTest(const FString& Parameters)
{
    // A hand-made line: start, a locked door, the goal. With no key the check fails, with the key behind the start it passes
    FNsDungeonGraph Graph;
    Graph.Reset(FIntPoint(3, 3));
    Graph.StartNode = 0;
    Graph.GoalNode = 2;
    Graph.Nodes[0].Role = ENsDungeonNodeRole::Start;
    Graph.Nodes[1].Role = ENsDungeonNodeRole::Path;
    Graph.Nodes[2].Role = ENsDungeonNodeRole::Goal;
    Graph.Connect(0, 1, ENsDungeonEdgeKind::Open);
    const int32 Lock = Graph.AddLock(Graph.Connect(1, 2, ENsDungeonEdgeKind::Open), TEXT("Door"));
    FString Message;
    TestFalse(TEXT("A lock with no key blocks the goal"), FNsDungeonGenerator::Validate(Graph, Message));
    Graph.Nodes[3].Role = ENsDungeonNodeRole::Path;
    Graph.Connect(0, 3, ENsDungeonEdgeKind::Open);
    Graph.AddKey(Lock, 3, TEXT("Key"));
    TestTrue(TEXT("Its key next to the start opens it"), FNsDungeonGenerator::Validate(Graph, Message));
    // A one-way door that opens only from the goal side blocks the way in
    Graph.Edges[Graph.FindEdge(1, 2)].Kind = ENsDungeonEdgeKind::OneWay;
    Graph.Edges[Graph.FindEdge(1, 2)].OneWayFrom = 2;
    TestFalse(TEXT("A one-way door from the goal side blocks the way in"), FNsDungeonGenerator::Validate(Graph, Message));
    return true;
}

#endif
