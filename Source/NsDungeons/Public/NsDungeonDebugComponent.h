// Copyright (C) 2025 nulled.softworks. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "NsDungeonGenerator.h"
#include "NsDungeonDebugComponent.generated.h"

/**
 * The play overlay of NsDungeons: draws the layout of a dungeon over the floor that the game built, in the space of the owner actor.
 * Rooms show as boxes, doors in the colors of their kind, with the locks, the keys, the start, and the goal as labels. The console
 * variable NsDungeons.ShowDebug turns it on. Non-shipping builds only
 */
UCLASS(ClassGroup = (NsDungeons), meta = (BlueprintSpawnableComponent))
class NSDUNGEONS_API UNsDungeonDebugComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UNsDungeonDebugComponent();

    //~ Begin UActorComponent
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
    //~ End UActorComponent

    /** Sets the dungeon to draw, and the size of a cell in cm. Cell (0, 0) starts at the owner origin */
    UFUNCTION(BlueprintCallable, Category = "NsDungeons")
    void SetDungeon(const FNsDungeonResult& InDungeon, float InCellSize = 100.0f);

private:
    /** Returns the world point over the middle of a cell, at a height over the floor */
    FVector GetCellPoint(const FIntPoint& Cell, double Height) const;

private:
    /** The dungeon to draw */
    FNsDungeonResult Dungeon;

    /** The size of a cell in cm */
    float CellSize = 100.0f;
};
