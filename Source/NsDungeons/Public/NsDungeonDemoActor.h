// Copyright (C) 2025 nulled.softworks. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "NsDungeonGenerator.h"
#include "NsDungeonDemoActor.generated.h"

/**
 * The NsDungeons demo: builds a greybox floor from a rule set and a seed, in the editor viewport and in play. Room floors, corridor tiles,
 * walls, and door posts in the colors of their kind are plain cubes, and the play overlay labels the start, the goal, the keys, and the
 * locks. In play: N next seed, P previous seed, R random seed, 1 to 9 floats the graph of that step over the floor, 0 hides it. It is a
 * showcase and a test bench: a game builds its own floor from the layout
 */
UCLASS()
class NSDUNGEONS_API ANsDungeonDemoActor : public AActor
{
    GENERATED_BODY()

public:
    ANsDungeonDemoActor();

    //~ Begin AActor
    virtual void OnConstruction(const FTransform& Transform) override;
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
    virtual void Tick(float DeltaSeconds) override;
    //~ End AActor

    /** Generates the dungeon of the seed and builds its floor */
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "NsDungeons")
    void Rebuild();

    /** Moves to another seed and rebuilds */
    UFUNCTION(BlueprintCallable, Category = "NsDungeons")
    void ChangeSeed(int32 Step);

    /** Floats the graph of a step over the floor, or hides it with INDEX_NONE */
    UFUNCTION(BlueprintCallable, Category = "NsDungeons")
    void ShowStep(int32 Step);

protected:
    /** The rule set, or none for the default settings */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "NsDungeons")
    TObjectPtr<class UNsDungeonRuleSet> RuleSet;

    /** The seed of the dungeon */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "NsDungeons")
    int32 Seed = 7;

    /** The size of a cell in cm */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "NsDungeons", meta = (ClampMin = "10.0"))
    float CellSize = 100.0f;

    /** The height of the walls in cm */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "NsDungeons", meta = (ClampMin = "10.0"))
    float WallHeight = 150.0f;

    /** The scene root */
    UPROPERTY(VisibleAnywhere, Category = "NsDungeons")
    TObjectPtr<class USceneComponent> Root;

    /** The camera over the floor */
    UPROPERTY(VisibleAnywhere, Category = "NsDungeons")
    TObjectPtr<class UCameraComponent> Camera;

    /** The play overlay */
    UPROPERTY(VisibleAnywhere, Category = "NsDungeons")
    TObjectPtr<class UNsDungeonDebugComponent> Overlay;

    /** The material of the cubes. Each kind of piece gets its own color from it */
    UPROPERTY()
    TObjectPtr<class UMaterialInterface> ShapeMaterial;

    /** The cube mesh of every piece */
    UPROPERTY()
    TObjectPtr<class UStaticMesh> CubeMesh;

private:
    /** Adds one cube piece: a location of its middle, a size in cm, and a color */
    void AddPiece(const FVector& Location, const FVector& Size, const FLinearColor& Color);

    /** Removes every piece of the last build */
    void ClearPieces();

    /** Draws the graph of the shown step over the floor */
    void DrawStep() const;

    /** Binds the demo keys in play */
    void BindKeys();

    /** Returns the color of a door kind: grey open, red locked, orange one-way, purple hidden */
    static FLinearColor GetDoorColor(ENsDungeonEdgeKind Kind);

private:
    /** The dungeon of the seed */
    FNsDungeonResult Dungeon;

    /** A material for each color of the last build */
    UPROPERTY(Transient)
    TArray<TObjectPtr<class UMaterialInstanceDynamic>> Paints;

    /** The color of each material, in the same order */
    TArray<FLinearColor> PaintColors;

    /** The step that floats over the floor, or INDEX_NONE */
    int32 ShownStep = INDEX_NONE;

    /** The value of NsDungeons.ShowDebug before play, given back at the end */
    int32 OverlayBefore = 0;

    /** The tag of the pieces that the demo builds */
    static inline const FName PieceTag = TEXT("NsDungeonsDemoPiece");

    /** The colors of the pieces: stone room floors, darker corridors, and dark walls */
    static inline const FLinearColor RoomFloorColor = FLinearColor(0.3f, 0.3f, 0.32f);
    static inline const FLinearColor CorridorFloorColor = FLinearColor(0.2f, 0.2f, 0.22f);
    static inline const FLinearColor WallColor = FLinearColor(0.1f, 0.1f, 0.12f);
};
