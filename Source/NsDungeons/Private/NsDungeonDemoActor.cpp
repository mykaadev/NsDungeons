// Copyright (C) 2025 nulled.softworks. All rights reserved.

#include "NsDungeonDemoActor.h"

#include "Camera/CameraComponent.h"
#include "Components/InputComponent.h"
#include "Components/StaticMeshComponent.h"
#include "DrawDebugHelpers.h"
#include "Engine/Engine.h"
#include "Engine/StaticMesh.h"
#include "GameFramework/PlayerController.h"
#include "HAL/IConsoleManager.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Misc/CommandLine.h"
#include "Misc/Paths.h"
#include "TimerManager.h"
#include "UnrealClient.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "NsDungeonDebugComponent.h"
#include "NsDungeonRuleSet.h"
#include "UObject/ConstructorHelpers.h"

ANsDungeonDemoActor::ANsDungeonDemoActor()
{
    PrimaryActorTick.bCanEverTick = true;
    Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
    RootComponent = Root;
    static ConstructorHelpers::FObjectFinder<UMaterialInterface> Material(TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial"));
    ShapeMaterial = Material.Object;

    static ConstructorHelpers::FObjectFinder<UStaticMesh> Cube(TEXT("/Engine/BasicShapes/Cube.Cube"));
    CubeMesh = Cube.Object;

    Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
    Camera->SetupAttachment(Root);
    // A fixed exposure: auto exposure washes the piece colors out to white
    Camera->PostProcessSettings.bOverride_AutoExposureMinBrightness = true;
    Camera->PostProcessSettings.AutoExposureMinBrightness = 1.0f;
    Camera->PostProcessSettings.bOverride_AutoExposureMaxBrightness = true;
    Camera->PostProcessSettings.AutoExposureMaxBrightness = 1.0f;
    Camera->PostProcessSettings.bOverride_AutoExposureBias = true;
    Camera->PostProcessSettings.AutoExposureBias = 0.0f;
    Overlay = CreateDefaultSubobject<UNsDungeonDebugComponent>(TEXT("Overlay"));
}

FLinearColor ANsDungeonDemoActor::GetDoorColor(const ENsDungeonEdgeKind Kind)
{
    switch (Kind)
    {
    case ENsDungeonEdgeKind::Locked: return FLinearColor(0.9f, 0.15f, 0.1f);
    case ENsDungeonEdgeKind::OneWay: return FLinearColor(1.0f, 0.55f, 0.1f);
    case ENsDungeonEdgeKind::Hidden: return FLinearColor(0.6f, 0.3f, 0.9f);
    default: return FLinearColor(0.75f, 0.75f, 0.75f);
    }
}

void ANsDungeonDemoActor::AddPiece(const FVector& Location, const FVector& Size, const FLinearColor& Color)
{
    // A plain mesh piece: the basic material does not work on instanced meshes. Pieces made in the construction script go away on the next one
    UStaticMeshComponent* const Piece = NewObject<UStaticMeshComponent>(this);
    Piece->CreationMethod = EComponentCreationMethod::UserConstructionScript;
    Piece->ComponentTags.Add(PieceTag);
    Piece->SetupAttachment(Root);
    Piece->SetStaticMesh(CubeMesh);
    Piece->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    Piece->SetRelativeLocation(Location);
    Piece->SetRelativeScale3D(Size / 100.0);
    // An undo in the editor can give back an older list of paints: then both lists start again
    if (Paints.Num() != PaintColors.Num())
    {
        Paints.Reset();
        PaintColors.Reset();
    }
    int32 Paint = PaintColors.IndexOfByKey(Color);
    if (Paint == INDEX_NONE || Paints[Paint] == nullptr)
    {
        if (Paint != INDEX_NONE)
        {
            Paints.RemoveAt(Paint);
            PaintColors.RemoveAt(Paint);
        }
        UMaterialInstanceDynamic* const Made = UMaterialInstanceDynamic::Create(ShapeMaterial, this);
        Made->SetVectorParameterValue(TEXT("Color"), Color);
        Paint = Paints.Add(Made);
        PaintColors.Add(Color);
    }
    Piece->SetMaterial(0, Paints[Paint]);
    Piece->RegisterComponent();
}

void ANsDungeonDemoActor::ClearPieces()
{
    TInlineComponentArray<UStaticMeshComponent*> Meshes(this);
    for (UStaticMeshComponent* const Mesh : Meshes)
    {
        if (Mesh->ComponentHasTag(PieceTag))
        {
            Mesh->DestroyComponent();
        }
    }
}

void ANsDungeonDemoActor::OnConstruction(const FTransform& Transform)
{
    Super::OnConstruction(Transform);
    Rebuild();
}

void ANsDungeonDemoActor::BeginPlay()
{
    Super::BeginPlay();
    // A spawn in play already built the dungeon in OnConstruction: build only when no dungeon is there
    if (Dungeon.Layout.Rooms.IsEmpty())
    {
        Rebuild();
    }
    // The demo takes the view, shows the overlay, and listens to its keys
    if (APlayerController* const Player = UGameplayStatics::GetPlayerController(this, 0))
    {
        Player->SetViewTarget(this);
        Player->bShowMouseCursor = true;
    }
    if (IConsoleVariable* const ShowDebug = IConsoleManager::Get().FindConsoleVariable(TEXT("NsDungeons.ShowDebug")))
    {
        OverlayBefore = ShowDebug->GetInt();
        ShowDebug->Set(1, ECVF_SetByConsole);
    }
    BindKeys();
#if !UE_BUILD_SHIPPING
    // Checks with no person: -NsDungeonsDemoStep=N shows a step, -NsDungeonsDemoShot=Seconds saves a screenshot and quits
    int32 Step = 0;
    if (FParse::Value(FCommandLine::Get(), TEXT("NsDungeonsDemoStep="), Step))
    {
        ShowStep(Step - 1);
    }
    float ShotTime = 0.0f;
    if (FParse::Value(FCommandLine::Get(), TEXT("NsDungeonsDemoShot="), ShotTime))
    {
        FTimerHandle Timer;
        GetWorldTimerManager().SetTimer(Timer, FTimerDelegate::CreateWeakLambda(this, [this]()
        {
            FScreenshotRequest::RequestScreenshot(FPaths::ProjectSavedDir() / TEXT("Screenshots/NsDungeonsDemo.png"), true, false);
            FTimerHandle Quit;
            GetWorldTimerManager().SetTimer(Quit, FTimerDelegate::CreateWeakLambda(this, [this]() { UKismetSystemLibrary::QuitGame(this, nullptr, EQuitPreference::Quit, false); }), 2.0f, false);
        }), FMath::Max(ShotTime, 0.5f), false);
    }
#endif
}

void ANsDungeonDemoActor::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    // The overlay setting goes back to what it was before play
    if (IConsoleVariable* const ShowDebug = IConsoleManager::Get().FindConsoleVariable(TEXT("NsDungeons.ShowDebug")))
    {
        ShowDebug->Set(OverlayBefore, ECVF_SetByConsole);
    }
    Super::EndPlay(EndPlayReason);
}

void ANsDungeonDemoActor::Rebuild()
{
    const FNsDungeonSettings Settings = RuleSet != nullptr ? RuleSet->Settings : FNsDungeonSettings();
    Dungeon = FNsDungeonGenerator::Generate(Settings, Seed, true);
    Seed = Dungeon.Seed;
    ClearPieces();

    // The basic cube is 1 m on each side, with its pivot in the middle
    const FNsDungeonLayout& Layout = Dungeon.Layout;
    const double Cell = CellSize;
    const auto Middle = [Cell](const FIntPoint& Point) { return FVector((Point.X + 0.5) * Cell, (Point.Y + 0.5) * Cell, 0.0); };
    // One slab for each room, one tile for each corridor cell
    for (const FNsDungeonLayoutRoom& Room : Layout.Rooms)
    {
        const FVector Size(Room.Size.X * Cell, Room.Size.Y * Cell, 10.0);
        AddPiece(FVector(Room.Min.X * Cell, Room.Min.Y * Cell, -5.0) + FVector(Size.X, Size.Y, 0.0) * 0.5, Size - FVector(4.0, 4.0, 0.0), RoomFloorColor);
    }
    for (int32 Y = 0; Y < Layout.Size.Y; ++Y)
    {
        for (int32 X = 0; X < Layout.Size.X; ++X)
        {
            const ENsDungeonCell Kind = Layout.GetCell(FIntPoint(X, Y));
            if (Kind == ENsDungeonCell::Corridor || Kind == ENsDungeonCell::Door)
            {
                AddPiece(Middle(FIntPoint(X, Y)) - FVector(0.0, 0.0, 5.0), FVector(Cell - 4.0, Cell - 4.0, 10.0), CorridorFloorColor);
            }
        }
    }
    // The walls: each run of cell sides that face rock becomes one long wall
    for (const FIntPoint& Side : { FIntPoint(1, 0), FIntPoint(-1, 0), FIntPoint(0, 1), FIntPoint(0, -1) })
    {
        const bool bAlongY = Side.X != 0;
        const int32 Lines = bAlongY ? Layout.Size.X : Layout.Size.Y;
        const int32 Length = bAlongY ? Layout.Size.Y : Layout.Size.X;
        for (int32 Line = 0; Line < Lines; ++Line)
        {
            int32 RunStart = INDEX_NONE;
            for (int32 Along = 0; Along <= Length; ++Along)
            {
                const FIntPoint Point = bAlongY ? FIntPoint(Line, Along) : FIntPoint(Along, Line);
                const bool bWall = Along < Length && Layout.GetCell(Point) != ENsDungeonCell::Empty && Layout.GetCell(Point + Side) == ENsDungeonCell::Empty;
                if (bWall && RunStart == INDEX_NONE)
                {
                    RunStart = Along;
                }
                if (!bWall && RunStart != INDEX_NONE)
                {
                    const double Run = (Along - RunStart) * Cell;
                    const FIntPoint First = bAlongY ? FIntPoint(Line, RunStart) : FIntPoint(RunStart, Line);
                    const FVector Start = Middle(First) + FVector(Side.X, Side.Y, 0.0) * Cell * 0.5;
                    const FVector Location = Start + (bAlongY ? FVector(0.0, Run * 0.5 - Cell * 0.5, 0.0) : FVector(Run * 0.5 - Cell * 0.5, 0.0, 0.0)) + FVector(0.0, 0.0, WallHeight * 0.5);
                    AddPiece(Location, bAlongY ? FVector(12.0, Run + 12.0, WallHeight) : FVector(Run + 12.0, 12.0, WallHeight), WallColor);
                    RunStart = INDEX_NONE;
                }
            }
        }
    }
    // A door: two posts at the sides of its cell, across the corridor, and a bar over a closed door
    for (const FNsDungeonLayoutDoor& Door : Layout.Doors)
    {
        const FLinearColor Color = GetDoorColor(Door.Kind);
        const FVector Across(Door.Direction.Y, Door.Direction.X, 0.0);
        for (const double Post : { -0.45, 0.45 })
        {
            AddPiece(Middle(Door.Cell) + Across * Post * Cell + FVector(0.0, 0.0, WallHeight * 0.6), FVector(14.0, 14.0, WallHeight * 1.2), Color);
        }
        if (Door.Kind != ENsDungeonEdgeKind::Open)
        {
            AddPiece(Middle(Door.Cell) + FVector(0.0, 0.0, WallHeight * 1.2), Across.X != 0.0 ? FVector(Cell, 10.0, 10.0) : FVector(10.0, Cell, 10.0), Color);
        }
    }

    // The camera looks down on the whole floor
    const FVector Center(Layout.Size.X * Cell * 0.5, Layout.Size.Y * Cell * 0.5, 0.0);
    const FRotator View(-65.0, 0.0, 0.0);
    const double Distance = FMath::Max(Layout.Size.X, Layout.Size.Y) * Cell * 1.15;
    Camera->SetRelativeLocationAndRotation(Center - View.Vector() * Distance, View);
    Overlay->SetDungeon(Dungeon, CellSize);
}

void ANsDungeonDemoActor::ChangeSeed(const int32 Step)
{
    Seed = FMath::Max(Seed + Step, 0);
    Rebuild();
}

void ANsDungeonDemoActor::ShowStep(const int32 Step)
{
    ShownStep = Dungeon.Steps.IsValidIndex(Step) ? Step : INDEX_NONE;
}

void ANsDungeonDemoActor::BindKeys()
{
    APlayerController* const Player = UGameplayStatics::GetPlayerController(this, 0);
    if (Player == nullptr)
    {
        return;
    }
    EnableInput(Player);
    const auto Bind = [this](const FKey& Key, TFunction<void()> Run)
    {
        FInputKeyBinding Binding(FInputChord(Key), IE_Pressed);
        Binding.KeyDelegate.GetDelegateForManualSet().BindLambda(MoveTemp(Run));
        InputComponent->KeyBindings.Add(Binding);
    };
    Bind(EKeys::N, [this]() { ChangeSeed(1); });
    Bind(EKeys::P, [this]() { ChangeSeed(-1); });
    Bind(EKeys::R, [this]() { Seed = FMath::Rand(); Rebuild(); });
    Bind(EKeys::Zero, [this]() { ShowStep(INDEX_NONE); });
    const FKey Numbers[] = { EKeys::One, EKeys::Two, EKeys::Three, EKeys::Four, EKeys::Five, EKeys::Six, EKeys::Seven, EKeys::Eight, EKeys::Nine };
    for (int32 Index = 0; Index < 9; ++Index)
    {
        Bind(Numbers[Index], [this, Index]() { ShowStep(Index); });
    }
}

void ANsDungeonDemoActor::Tick(const float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);
#if !UE_BUILD_SHIPPING
    if (GEngine != nullptr && GetWorld()->IsGameWorld())
    {
        const FString Step = Dungeon.Steps.IsValidIndex(ShownStep)
            ? FString::Printf(TEXT("Graph of step %d: %s. %s"), ShownStep + 1, *Dungeon.Steps[ShownStep].Name.ToString(), *Dungeon.Steps[ShownStep].Note) : TEXT("Graph hidden");
        GEngine->AddOnScreenDebugMessage(9100, 0.0f, FColor::White, FString::Printf(TEXT("NsDungeons demo   Seed %d   %s"), Seed, *Dungeon.Message));
        GEngine->AddOnScreenDebugMessage(9101, 0.0f, FColor(200, 200, 200), FString::Printf(TEXT("N next seed   P previous seed   R random seed   1 to %d graph of a step   0 hide it   %s"),
            Dungeon.Steps.Num(), *Step));
    }
#endif
    DrawStep();
}

void ANsDungeonDemoActor::DrawStep() const
{
#if ENABLE_DRAW_DEBUG
    if (!Dungeon.Steps.IsValidIndex(ShownStep))
    {
        return;
    }
    // The graph floats 4 m over the floor: each node over the middle of its block
    const FNsDungeonSettings Settings = RuleSet != nullptr ? RuleSet->Settings : FNsDungeonSettings();
    const int32 Block = FMath::Max3(Settings.RoomCellsMax, Settings.RoomCellsMin, 1);
    const int32 Stride = Block + FMath::Max(Settings.CorridorCells, 1);
    const FNsDungeonGraph& Graph = Dungeon.Steps[ShownStep].Graph;
    const auto Spot = [this, &Graph, Block, Stride](const int32 Node)
    {
        const FIntPoint Coord = Graph.Nodes[Node].Coord;
        return GetActorTransform().TransformPosition(FVector((Coord.X * Stride + Block * 0.5) * CellSize, (Coord.Y * Stride + Block * 0.5) * CellSize, 400.0));
    };
    for (const FNsDungeonEdge& Edge : Graph.Edges)
    {
        const FColor Color = Edge.Kind == ENsDungeonEdgeKind::Locked ? FColor(230, 40, 25) : Edge.Kind == ENsDungeonEdgeKind::OneWay ? FColor(255, 140, 25)
            : Edge.Kind == ENsDungeonEdgeKind::Hidden ? FColor(150, 75, 230) : FColor(200, 200, 200);
        DrawDebugLine(GetWorld(), Spot(Edge.NodeA), Spot(Edge.NodeB), Color, false, -1.0f, 1, 8.0f);
    }
    for (int32 Node = 0; Node < Graph.Nodes.Num(); ++Node)
    {
        const ENsDungeonNodeRole NodeRole = Graph.Nodes[Node].Role;
        if (NodeRole == ENsDungeonNodeRole::Unused)
        {
            DrawDebugPoint(GetWorld(), Spot(Node), 6.0f, FColor(60, 60, 70), false, -1.0f, 1);
            continue;
        }
        const FColor Color = NodeRole == ENsDungeonNodeRole::Start ? FColor::Green : NodeRole == ENsDungeonNodeRole::Goal ? FColor(255, 200, 40)
            : Graph.Nodes[Node].bMinorCycle ? FColor(70, 110, 220) : FColor(170, 170, 180);
        DrawDebugSolidBox(GetWorld(), Spot(Node), FVector(CellSize * 0.6), Color, false, -1.0f, 1);
    }
    for (const FNsDungeonKey& Key : Graph.Keys)
    {
        DrawDebugString(GetWorld(), Spot(Key.Node) + FVector(0.0, 0.0, 90.0), FString::Printf(TEXT("KEY %d"), Key.LockId), nullptr, FColor(255, 215, 50), 0.0f, true);
    }
#endif
}
