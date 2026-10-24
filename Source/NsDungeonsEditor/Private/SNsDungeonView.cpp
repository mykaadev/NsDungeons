// Copyright (C) 2025 nulled.softworks. All rights reserved.

#include "SNsDungeonView.h"

#include "Rendering/DrawElements.h"
#include "Styling/AppStyle.h"
#include "Styling/CoreStyle.h"

void SNsDungeonView::Construct(const FArguments& InArgs)
{
    Result = InArgs._Result;
    StepIndex = InArgs._StepIndex;
    bShowLayout = InArgs._bShowLayout;
}

FLinearColor SNsDungeonView::GetEdgeColor(const ENsDungeonEdgeKind Kind)
{
    switch (Kind)
    {
    case ENsDungeonEdgeKind::Locked: return FLinearColor(0.9f, 0.15f, 0.1f);
    case ENsDungeonEdgeKind::OneWay: return FLinearColor(1.0f, 0.55f, 0.1f);
    case ENsDungeonEdgeKind::Hidden: return FLinearColor(0.6f, 0.3f, 0.9f);
    default: return FLinearColor(0.75f, 0.75f, 0.75f);
    }
}

int32 SNsDungeonView::OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements,
    int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const
{
    PaintBox(AllottedGeometry, OutDrawElements, LayerId, FVector2D::ZeroVector, AllottedGeometry.GetLocalSize(), FLinearColor(0.02f, 0.02f, 0.025f));
    if (!Result.IsValid())
    {
        return LayerId + 1;
    }
    return bShowLayout ? PaintLayout(AllottedGeometry, OutDrawElements, LayerId + 1) : PaintGraph(AllottedGeometry, OutDrawElements, LayerId + 1);
}

int32 SNsDungeonView::PaintGraph(const FGeometry& Geometry, FSlateWindowElementList& OutDrawElements, const int32 LayerId) const
{
    const int32 Step = StepIndex.Get();
    const FNsDungeonGraph& Graph = Result->Steps.IsValidIndex(Step) ? Result->Steps[Step].Graph : Result->Graph;
    const FVector2D Area = Geometry.GetLocalSize();
    const double Cell = FMath::Min(Area.X / Graph.Size.X, Area.Y / Graph.Size.Y);
    const FVector2D Origin = (Area - FVector2D(Graph.Size.X, Graph.Size.Y) * Cell) * 0.5;
    const auto Center = [&Graph, Cell, Origin](const int32 Node)
    {
        return Origin + (FVector2D(Graph.Nodes[Node].Coord.X, Graph.Nodes[Node].Coord.Y) + 0.5) * Cell;
    };

    // The empty grid, faint
    for (int32 Node = 0; Node < Graph.Nodes.Num(); ++Node)
    {
        PaintBox(Geometry, OutDrawElements, LayerId, Center(Node) - FVector2D(2.0), FVector2D(4.0), FLinearColor(0.15f, 0.15f, 0.17f));
    }
    // The edges: the color tells the kind, a dot near one end shows the open side of a one-way door
    for (const FNsDungeonEdge& Edge : Graph.Edges)
    {
        const FVector2D A = Center(Edge.NodeA);
        const FVector2D B = Center(Edge.NodeB);
        FSlateDrawElement::MakeLines(OutDrawElements, LayerId + 1, Geometry.ToPaintGeometry(), { A, B }, ESlateDrawEffect::None, GetEdgeColor(Edge.Kind), true,
            Edge.Kind == ENsDungeonEdgeKind::Open ? 2.0f : 4.0f);
        if (Edge.Kind == ENsDungeonEdgeKind::OneWay && Graph.Nodes.IsValidIndex(Edge.OneWayFrom))
        {
            const FVector2D From = Center(Edge.OneWayFrom);
            const FVector2D To = Center(Edge.GetOther(Edge.OneWayFrom));
            PaintBox(Geometry, OutDrawElements, LayerId + 2, FMath::Lerp(From, To, 0.7) - FVector2D(5.0), FVector2D(10.0), GetEdgeColor(Edge.Kind));
        }
        if (Edge.Kind == ENsDungeonEdgeKind::Locked)
        {
            PaintText(Geometry, OutDrawElements, LayerId + 3, (A + B) * 0.5 + FVector2D(4.0, -14.0), FString::Printf(TEXT("L%d"), Edge.LockId), GetEdgeColor(Edge.Kind), 9);
        }
    }
    // The rooms: the role and the side loop tell the color, a ring shows a dead end
    const double Room = Cell * 0.42;
    for (int32 Node = 0; Node < Graph.Nodes.Num(); ++Node)
    {
        const FNsDungeonNode& Data = Graph.Nodes[Node];
        if (Data.Role == ENsDungeonNodeRole::Unused)
        {
            continue;
        }
        FLinearColor Color = Data.bMinorCycle ? FLinearColor(0.2f, 0.35f, 0.6f) : FLinearColor(0.35f, 0.35f, 0.38f);
        Color = Data.Role == ENsDungeonNodeRole::Start ? FLinearColor(0.15f, 0.6f, 0.25f) : Data.Role == ENsDungeonNodeRole::Goal ? FLinearColor(0.85f, 0.65f, 0.15f) : Color;
        if (Data.bDeadEnd)
        {
            PaintBox(Geometry, OutDrawElements, LayerId + 4, Center(Node) - FVector2D(Room * 0.5 + 3.0), FVector2D(Room + 6.0), FLinearColor(0.08f, 0.08f, 0.09f));
        }
        PaintBox(Geometry, OutDrawElements, LayerId + 5, Center(Node) - FVector2D(Room * 0.5), FVector2D(Room), Color);
        FString Label = Data.Distance != INDEX_NONE ? FString::FromInt(Data.Distance) : FString();
        for (const FName& RoomTag : Data.Tags)
        {
            Label += TEXT(" ") + RoomTag.ToString();
        }
        PaintText(Geometry, OutDrawElements, LayerId + 6, Center(Node) - FVector2D(Room * 0.5 - 2.0, Room * 0.5 - 1.0), Label, FLinearColor::White, 8);
    }
    // The keys: a small gold box with the lock that it opens
    for (const FNsDungeonKey& Key : Graph.Keys)
    {
        const FVector2D Spot = Center(Key.Node) + FVector2D(Room * 0.15, Room * 0.1);
        PaintBox(Geometry, OutDrawElements, LayerId + 7, Spot, FVector2D(9.0), FLinearColor(1.0f, 0.85f, 0.2f));
        PaintText(Geometry, OutDrawElements, LayerId + 7, Spot + FVector2D(11.0, -3.0), FString::Printf(TEXT("K%d"), Key.LockId), FLinearColor(1.0f, 0.85f, 0.2f), 8);
    }
    return LayerId + 8;
}

int32 SNsDungeonView::PaintLayout(const FGeometry& Geometry, FSlateWindowElementList& OutDrawElements, const int32 LayerId) const
{
    const FNsDungeonLayout& Layout = Result->Layout;
    if (Layout.Size.X <= 0 || Layout.Size.Y <= 0)
    {
        return LayerId;
    }
    const FVector2D Area = Geometry.GetLocalSize();
    const double Cell = FMath::Min(Area.X / Layout.Size.X, Area.Y / Layout.Size.Y);
    const FVector2D Origin = (Area - FVector2D(Layout.Size.X, Layout.Size.Y) * Cell) * 0.5;
    for (int32 Y = 0; Y < Layout.Size.Y; ++Y)
    {
        for (int32 X = 0; X < Layout.Size.X; ++X)
        {
            const ENsDungeonCell Kind = Layout.GetCell(FIntPoint(X, Y));
            if (Kind == ENsDungeonCell::Empty)
            {
                continue;
            }
            FLinearColor Color = Kind == ENsDungeonCell::Room ? FLinearColor(0.32f, 0.32f, 0.35f) : FLinearColor(0.18f, 0.18f, 0.2f);
            if (Kind == ENsDungeonCell::Door)
            {
                const FNsDungeonLayoutDoor* const Door = Layout.FindDoorAt(FIntPoint(X, Y));
                Color = GetEdgeColor(Door != nullptr ? Door->Kind : ENsDungeonEdgeKind::Open);
            }
            PaintBox(Geometry, OutDrawElements, LayerId, Origin + FVector2D(X, Y) * Cell + FVector2D(0.5), FVector2D(Cell - 1.0), Color);
        }
    }
    // The start, the goal, and the keys over their rooms
    const FNsDungeonGraph& Graph = Result->Graph;
    for (const FNsDungeonLayoutRoom& Room : Layout.Rooms)
    {
        const FVector2D Min = Origin + FVector2D(Room.Min.X, Room.Min.Y) * Cell;
        if (Room.Node == Graph.StartNode || Room.Node == Graph.GoalNode)
        {
            PaintText(Geometry, OutDrawElements, LayerId + 1, Min + FVector2D(2.0), Room.Node == Graph.StartNode ? TEXT("START") : TEXT("GOAL"),
                Room.Node == Graph.StartNode ? FLinearColor(0.3f, 1.0f, 0.4f) : FLinearColor(1.0f, 0.8f, 0.2f), 8);
        }
        for (const FNsDungeonKey& Key : Graph.Keys)
        {
            if (Key.Node == Room.Node)
            {
                PaintText(Geometry, OutDrawElements, LayerId + 1, Min + FVector2D(2.0, Cell * Room.Size.Y - 14.0), FString::Printf(TEXT("K%d"), Key.LockId), FLinearColor(1.0f, 0.85f, 0.2f), 8);
            }
        }
    }
    return LayerId + 2;
}

void SNsDungeonView::PaintBox(const FGeometry& Geometry, FSlateWindowElementList& OutDrawElements, const int32 LayerId, const FVector2D& Position, const FVector2D& Size, const FLinearColor& Color)
{
    FSlateDrawElement::MakeBox(OutDrawElements, LayerId, Geometry.ToPaintGeometry(FVector2f(Size), FSlateLayoutTransform(FVector2f(Position))), FAppStyle::GetBrush("WhiteBrush"),
        ESlateDrawEffect::None, Color);
}

void SNsDungeonView::PaintText(const FGeometry& Geometry, FSlateWindowElementList& OutDrawElements, const int32 LayerId, const FVector2D& Position, const FString& Text,
    const FLinearColor& Color, const int32 FontSize)
{
    FSlateDrawElement::MakeText(OutDrawElements, LayerId, Geometry.ToPaintGeometry(FVector2f(200.0f, 20.0f), FSlateLayoutTransform(FVector2f(Position))), Text,
        FCoreStyle::GetDefaultFontStyle("Regular", FontSize), ESlateDrawEffect::None, Color);
}
