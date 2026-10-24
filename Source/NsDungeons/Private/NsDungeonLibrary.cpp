// Copyright (C) 2025 nulled.softworks. All rights reserved.

#include "NsDungeonLibrary.h"

FNsDungeonResult UNsDungeonLibrary::GenerateDungeon(const UNsDungeonRuleSet* RuleSet, const int32 Seed, const bool bKeepSteps)
{
    return FNsDungeonGenerator::Generate(RuleSet != nullptr ? RuleSet->Settings : FNsDungeonSettings(), Seed, bKeepSteps);
}

ENsDungeonCell UNsDungeonLibrary::GetLayoutCell(const FNsDungeonLayout& Layout, const FIntPoint Cell)
{
    return Layout.GetCell(Cell);
}

FVector UNsDungeonLibrary::CellToLocal(const FIntPoint Cell, const float CellSize)
{
    return FVector((Cell.X + 0.5) * CellSize, (Cell.Y + 0.5) * CellSize, 0.0);
}

FIntPoint UNsDungeonLibrary::LocalToCell(const FVector& Local, const float CellSize)
{
    const double Size = FMath::Max(static_cast<double>(CellSize), 1.0);
    return FIntPoint(FMath::FloorToInt32(Local.X / Size), FMath::FloorToInt32(Local.Y / Size));
}
