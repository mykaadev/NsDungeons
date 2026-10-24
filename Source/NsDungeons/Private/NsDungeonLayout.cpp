// Copyright (C) 2025 nulled.softworks. All rights reserved.

#include "NsDungeonLayout.h"

ENsDungeonCell FNsDungeonLayout::GetCell(const FIntPoint& Cell) const
{
    if (Cell.X < 0 || Cell.Y < 0 || Cell.X >= Size.X || Cell.Y >= Size.Y)
    {
        return ENsDungeonCell::Empty;
    }
    return Cells[Cell.Y * Size.X + Cell.X];
}

const FNsDungeonLayoutRoom* FNsDungeonLayout::FindRoomOfNode(const int32 Node) const
{
    return Rooms.FindByPredicate([Node](const FNsDungeonLayoutRoom& Room) { return Room.Node == Node; });
}

const FNsDungeonLayoutRoom* FNsDungeonLayout::FindRoomAt(const FIntPoint& Cell) const
{
    return Rooms.FindByPredicate([&Cell](const FNsDungeonLayoutRoom& Room) { return Room.Contains(Cell); });
}

const FNsDungeonLayoutDoor* FNsDungeonLayout::FindDoorAt(const FIntPoint& Cell) const
{
    return Doors.FindByPredicate([&Cell](const FNsDungeonLayoutDoor& Door) { return Door.Cell == Cell; });
}

void FNsDungeonLayout::SetCell(const FIntPoint& Cell, const ENsDungeonCell Kind)
{
    if (Cell.X >= 0 && Cell.Y >= 0 && Cell.X < Size.X && Cell.Y < Size.Y)
    {
        Cells[Cell.Y * Size.X + Cell.X] = Kind;
    }
}
