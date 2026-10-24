// Copyright (C) 2025 nulled.softworks. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "NsDungeonGenerator.h"
#include "Widgets/SLeafWidget.h"

/**
 * One drawing of the NsDungeons Preview: the graph of a step (rooms as boxes, edges as lines, locks, keys, one-way and hidden doors in
 * their colors), or the cell layout of the result. It reads a shared result and a step index, and paints them scaled to fit
 */
class SNsDungeonView : public SLeafWidget
{
public:
    SLATE_BEGIN_ARGS(SNsDungeonView) : _bShowLayout(false) {}
        /** The result to draw */
        SLATE_ARGUMENT(TSharedPtr<FNsDungeonResult>, Result)
        /** The step to draw, for the graph */
        SLATE_ATTRIBUTE(int32, StepIndex)
        /** True draws the cell layout, false draws the graph */
        SLATE_ARGUMENT(bool, bShowLayout)
    SLATE_END_ARGS()

    void Construct(const FArguments& InArgs);

    //~ Begin SWidget
    virtual int32 OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements,
        int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const override;
    virtual FVector2D ComputeDesiredSize(float LayoutScaleMultiplier) const override { return FVector2D(400.0, 400.0); }
    //~ End SWidget

    /** Returns the color of an edge kind, for the legend too */
    static FLinearColor GetEdgeColor(ENsDungeonEdgeKind Kind);

private:
    /** Paints the graph of the chosen step */
    int32 PaintGraph(const FGeometry& Geometry, FSlateWindowElementList& OutDrawElements, int32 LayerId) const;

    /** Paints the cell layout */
    int32 PaintLayout(const FGeometry& Geometry, FSlateWindowElementList& OutDrawElements, int32 LayerId) const;

    /** Paints a filled box */
    static void PaintBox(const FGeometry& Geometry, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FVector2D& Position, const FVector2D& Size, const FLinearColor& Color);

    /** Paints a short text */
    static void PaintText(const FGeometry& Geometry, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FVector2D& Position, const FString& Text, const FLinearColor& Color, int32 FontSize);

private:
    /** The result to draw */
    TSharedPtr<FNsDungeonResult> Result;

    /** The step to draw */
    TAttribute<int32> StepIndex;

    /** True draws the cell layout */
    bool bShowLayout = false;
};
