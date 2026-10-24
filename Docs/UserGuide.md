# NsDungeons User Guide

## Purpose

NsDungeons makes dungeon floors with a lock-and-key structure, after the method of Unexplored (Boris the Brave, 2021). It gives an abstract graph on a fixed grid (rooms, doors, locks, keys, one-way doors), and later a cell layout of that graph. An editor window shows each step of a generation.

NsDungeons does not build meshes, move characters, or find paths. The game builds the floor from the layout and owns its walk grid and its movement.

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

Blueprint: a Blueprint library follows in step D3.

## Common Tasks

| Task | How |
|---|---|
| Make a floor | `FNsDungeonGenerator::Generate(Settings, Seed)` |
| See each step | `Generate(Settings, Seed, true)`, then read `Result.Steps` |
| Check a hand-made graph | `FNsDungeonGenerator::Validate(Graph, Message)` |
| Find the door between two rooms | `Graph.FindEdge(NodeA, NodeB)`, then `Graph.Edges[Edge].Kind` |
| Find what a key opens | `Graph.Keys[Key].LockId`, then `Graph.Locks[LockId].Edge` |

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
| `LockKind`, `KeyKind` | `Door`, `Key` | The names on new locks and keys |
| `MaxSeedTries` | 20 | The tries with the next seeds before a failure |

## Major cycle types

| Type | Shape |
|---|---|
| `Simple` | Two routes to the goal, no lock |
| `LockAndKey` | The goal sits behind a lock. Its key waits at the end of the other route, which a one-way door closes off from the goal |
| `TwoKeys` | As `LockAndKey`, with a second key halfway along the locked route |
| `OneWayValve` | One route to the goal. The other route is a one-way way back from the goal |

## Debug

- Log category: `LogNsDungeons`. A seed that fails logs the seed that worked. A failure of every try logs an error with the reason.
- Tests: `Automation RunTests NsDungeons`.
- The editor preview window comes in step D4, and the play overlay in step D5.

## Limits

- Version 0.1: one major cycle, no minor cycles, no dead ends, no cell layout yet (steps D2 and D3).
- Only grid neighbors connect. A room has at most 4 doors.
