# NsDungeons User Guide

## Purpose

NsDungeons makes dungeon floors with a lock-and-key structure, after the method of Unexplored (Boris the Brave, 2021). It gives an abstract graph on a fixed grid (rooms, doors, locks, keys, one-way doors), and later a cell layout of that graph. An editor window shows each step of a generation.

NsDungeons does not build game floors, move characters, or find paths. The game builds its floor from the layout and owns its walk grid and its movement. The demo actor builds a plain greybox floor only to show the plugin.

## Setup

1. Add `NsDungeons` to the `Plugins` list of the `.uproject` file, with `"Enabled": true`.
2. Add `"NsDungeons"` to the dependencies of the game module that calls the generator.
3. Make a rule set asset: Content Browser, Miscellaneous, Data Asset, `NsDungeonRuleSet`.

## Quick Start

C++:

```cpp
#include "NsDungeonGenerator.h"

const FNsDungeonResult Result = FNsDungeonGenerator::Generate(RuleSet->Settings, Seed);
if (Result.bValid)
{
    for (const FNsDungeonNode& Node : Result.Graph.Nodes)
    {
        // Node.Role, Node.Coord, Node.Tags: one room for each used node
    }
}
```

Blueprint: `Generate Dungeon` (rule set, seed) returns the result with its graph and its layout. `Get Layout Cell`, `Cell To Local`, and `Local To Cell` read the cells.

## Common Tasks

| Task | How |
|---|---|
| Make a floor | `FNsDungeonGenerator::Generate(Settings, Seed)` |
| See each step | `Generate(Settings, Seed, true)`, then read `Result.Steps` |
| Check a hand-made graph | `FNsDungeonGenerator::Validate(Graph, Message)` |
| Find the door between two rooms | `Graph.FindEdge(NodeA, NodeB)`, then `Graph.Edges[Edge].Kind` |
| Find what a key opens | `Graph.Keys[Key].LockId`, then `Graph.Locks[LockId].Edge` |
| Find the camp room | Put a `RoomTags` entry `Camp` in the rule set, then look for the tag in `Node.Tags` |
| Build a floor | Read `Result.Layout`: `Cells` (Empty, Room, Corridor, Door), `Rooms` (node, first cell, size), `Doors` (cell, edge, kind, direction). One cell is one step of the walk grid of the game |
| Place a cell in the world | `UNsDungeonLibrary::CellToLocal(Cell, CellSize)`, then the transform of the floor actor |
| Know how deep a room is | `Node.Distance`: the doors from the start. `Node.bDeadEnd`, `Node.bMinorCycle` |

## Rules

- The same settings and the same seed always give the same dungeon. Keep the seed to rebuild a floor.
- A seed that gives no valid dungeon tries the next seeds. Read `Result.Seed` for the seed that worked.
- Nodes never move. Steps only mark them, so `Node.Coord` is the place of the room on the grid.
- Lock and key kinds are names. The game decides what a `Door` or a `Key` looks like.

## Settings

| Setting | Default | Effect |
|---|---|---|
| `GridSize` | 5 by 5 | The node grid. Each used node is a room |
| `MinStartGoalDistance` | 3 | The smallest grid distance from the start to the goal |
| `MajorCycles` | LockAndKey 2, TwoKeys 1, OneWayValve 1, Simple 1 | The major cycle types and their weights |
| `MinorCycles` | 1 | Side loops after the major cycle |
| `MinorLockChance` | 0.5 | The chance of a locked side loop: a lock at its entry, a one-way door out, and the key in a room that the party reaches first |
| `HiddenShortcutChance` | 0.5 | The chance of a hidden door between two rooms side by side that are far apart on the way |
| `TargetRooms` | 12 | Dead ends grow the floor to this many rooms, as far as the grid allows |
| `RoomTags` | none | Tags for the game to put on rooms, each with a count and a place: `Any`, `DeadEnd`, `MajorCycle`, `NearGoal`, `Far` |
| `RoomCellsMin`, `RoomCellsMax` | 3, 5 | The room size in cells on each side. Each room takes a random size between them |
| `CorridorCells` | 2 | The rock between two blocks of rooms: the shortest corridor |
| `LockKind`, `KeyKind` | `Door`, `Key` | The names on new locks and keys |
| `MaxSeedTries` | 20 | The tries with the next seeds before a failure |

## Major cycle types

| Type | Shape |
|---|---|
| `Simple` | Two routes to the goal, no lock |
| `LockAndKey` | The goal sits behind a lock. Its key waits at the end of the other route, which a one-way door closes off from the goal |
| `TwoKeys` | As `LockAndKey`, with a second key halfway along the locked route |
| `OneWayValve` | One route to the goal. The other route is a one-way way back from the goal |

## Demo

Open the map `/NsDungeons/Demo/L_NsDungeonsDemo` (Content Browser, Plugins, NsDungeons Content) and press Play. `NsDungeonDemoActor` builds a greybox floor from its rule set and seed: room floors, corridors, walls, and door posts in the colors of their kind, with the play overlay on.

| Key | Effect |
|---|---|
| N, P | Next seed, previous seed |
| R | A random seed |
| 1 to 9 | The graph of that step floats over the floor |
| 0 | Hides the graph |

Change `Seed` or `RuleSet` in the details panel: the floor rebuilds in the editor viewport. The map is made by `Scripts/make_demo_map.py`. The demo actor is a showcase and a test bench, not a floor builder for a game. Leave the `Demo` folder out of a cook (Project Settings, Packaging, Directories to never cook) when a game ships with NsDungeons.

## Debug

- Log category: `LogNsDungeons`. A seed that fails logs the seed that worked. A failure of every try logs an error with the reason.
- Tests: `Automation RunTests NsDungeons`.
- **NsDungeons Preview**: Tools, NsDungeons Preview. Pick a rule set (none uses the default settings) and a seed, then Generate, Next seed, or Random seed. The Step slider steps through the generation. The graph of the step shows on the left, and the cell layout on the right. The text gives the step, the validation result, and the seed that worked. The legend under the views gives the colors.
- `NsDungeons.SavePreview <Seed> <File.png>` (editor console): saves a picture of the preview for a seed, with the default settings.
- **Play overlay**: add a `NsDungeonDebugComponent` to the floor actor and call `SetDungeon(Result, CellSize)`. The console variable `NsDungeons.ShowDebug 1` then draws the rooms, the doors in their kind colors, and labels for the start, the goal, the tags, the keys, and the locks. Cell (0, 0) starts at the actor origin. Non-shipping builds only.

## Limits

- Version 0.5: corridors are straight, and rooms sit in the middle of their block. The walk grid, the doors that open, and the paths are game code.
- One tag for each room. A tag with no free room of its place goes on any free room.
- Only grid neighbors connect. A room has at most 4 doors.
