// Copyright (C) 2025 nulled.softworks. All rights reserved.

#include "NsDungeonDebugComponent.h"

#include "DrawDebugHelpers.h"
#include "GameFramework/Actor.h"
#include "HAL/IConsoleManager.h"

static TAutoConsoleVariable<int32> CVarNsDungeonsShowDebug(TEXT("NsDungeons.ShowDebug"), 0,
    TEXT("Draws the dungeon layout over the floor: 1 on, 0 off. Needs an NsDungeonDebugComponent with a dungeon"));

UNsDungeonDebugComponent::UNsDungeonDebugComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
}

void UNsDungeonDebugComponent::SetDungeon(const FNsDungeonResult& InDungeon, const float InCellSize)
{
    Dungeon = InDungeon;
    CellSize = FMath::Max(InCellSize, 1.0f);
}

FVector UNsDungeonDebugComponent::GetCellPoint(const FIntPoint& Cell, const double Height) const
{
    return GetOwner()->GetActorTransform().TransformPosition(FVector((Cell.X + 0.5) * CellSize, (Cell.Y + 0.5) * CellSize, Height));
}

void UNsDungeonDebugComponent::TickComponent(const float DeltaTime, const ELevelTick TickType, FActorComponentTickFunction* const ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
#if ENABLE_DRAW_DEBUG
    if (CVarNsDungeonsShowDebug.GetValueOnGameThread() == 0 || GetOwner() == nullptr)
    {
        return;
    }
    const UWorld* const World = GetWorld();
    const FNsDungeonGraph& Graph = Dungeon.Graph;
    const FNsDungeonLayout& Layout = Dungeon.Layout;
    const FQuat Rotation = GetOwner()->GetActorQuat();
    // The rooms: a flat box over their cells, green for the start, gold for the goal
    for (const FNsDungeonLayoutRoom& Room : Layout.Rooms)
    {
        const FVector Center = GetOwner()->GetActorTransform().TransformPosition(FVector((Room.Min.X + Room.Size.X * 0.5) * CellSize, (Room.Min.Y + Room.Size.Y * 0.5) * CellSize, 10.0));
        const FColor Color = Room.Node == Graph.StartNode ? FColor::Green : Room.Node == Graph.GoalNode ? FColor(255, 200, 40) : FColor(150, 150, 160);
        DrawDebugBox(World, Center, FVector(Room.Size.X * CellSize * 0.5, Room.Size.Y * CellSize * 0.5, 5.0), Rotation, Color, false, -1.0f, 0, 3.0f);
        FString Label = Room.Node == Graph.StartNode ? TEXT("START") : Room.Node == Graph.GoalNode ? TEXT("GOAL") : FString();
        for (const FName& RoomTag : Graph.Nodes[Room.Node].Tags)
        {
            Label += TEXT(" ") + RoomTag.ToString();
        }
        for (const FNsDungeonKey& Key : Graph.Keys)
        {
            Label += Key.Node == Room.Node ? FString::Printf(TEXT(" KEY %d"), Key.LockId) : FString();
        }
        if (!Label.IsEmpty())
        {
            DrawDebugString(World, Center + FVector(0.0, 0.0, 150.0), Label, nullptr, Color, 0.0f, true);
        }
    }
    // The doors: a post in the color of the kind
    for (const FNsDungeonLayoutDoor& Door : Layout.Doors)
    {
        FColor Color(190, 190, 190);
        FString Label;
        switch (Door.Kind)
        {
        case ENsDungeonEdgeKind::Locked: Color = FColor(230, 40, 25); Label = FString::Printf(TEXT("LOCK %d"), Graph.Edges.IsValidIndex(Door.Edge) ? Graph.Edges[Door.Edge].LockId : INDEX_NONE); break;
        case ENsDungeonEdgeKind::OneWay: Color = FColor(255, 140, 25); Label = TEXT("ONE-WAY"); break;
        case ENsDungeonEdgeKind::Hidden: Color = FColor(150, 75, 230); Label = TEXT("HIDDEN"); break;
        default: break;
        }
        DrawDebugLine(World, GetCellPoint(Door.Cell, 0.0), GetCellPoint(Door.Cell, 250.0), Color, false, -1.0f, 0, 6.0f);
        if (!Label.IsEmpty())
        {
            DrawDebugString(World, GetCellPoint(Door.Cell, 270.0), Label, nullptr, Color, 0.0f, true);
        }
    }
#endif
}
