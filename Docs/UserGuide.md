# NsDungeons User Guide

## Purpose

NsDungeons makes dungeon floors with a lock-and-key structure, after the cyclic method of Unexplored (Boris the Brave, 2021). It gives an abstract graph on a fixed grid (rooms, doors, locks, keys, one-way doors, and hidden doors) and a cell layout of that graph. An editor tab shows each step of a generation.

NsDungeons does not build game floors, move characters, or find paths. Your game builds its floor from the layout and owns its walk grid and its movement. The demo actor builds a plain greybox floor only to show the plugin.

[![The graph of seed 7 after the last step of a generation](Showcase/img/step8.jpg)](Showcase/GenerationSteps.html)

## Setup

1. Copy the `NsDungeons` folder into the `Plugins` folder of your project.
2. Open the project and enable NsDungeons in Edit > Plugins.
3. For C++, add `"NsDungeons"` to the dependencies of the game module that calls the generator.
4. Generate the project files and build the project.

The modules build for Windows (Win64) only. No project settings are necessary.

## Quick Start

### Blueprint

1. In the Content Browser, select Add > Miscellaneous > Data Asset.
2. Pick the class `NsDungeonRuleSet` and name the asset.
3. In a Blueprint, call `Generate Dungeon` with the rule set and a seed.
4. Read **Graph** and **Layout** from the result.
5. Use `Get Layout Cell` and `Cell To Local` to place floor pieces.

### C++

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

### Demo map

1. In the Content Browser, open Plugins > NsDungeons Content > Demo. Enable **Show Plugin Content** in the view settings if the folder is not visible.
2. Open the map `L_NsDungeonsDemo` (`/NsDungeons/Demo/L_NsDungeonsDemo`).
3. Click Play.

The demo actor builds a greybox floor from its rule set and seed: room floors, corridors, walls, and door posts in the colors of their kind. The play overlay is on during play.

| Key | Effect |
|---|---|
| N | Next seed |
| P | Previous seed (not below 0) |
| R | A random seed |
| 1 to 8 | Shows the graph of that step over the floor |
| 9, 0 | Hides the graph |

A generation has 8 steps, so the 9 key shows no step.

## Common Tasks

### Make a floor from a seed

1. Call `FNsDungeonGenerator::Generate(Settings, Seed)`, or `Generate Dungeon` in Blueprint.
2. Make sure that `bValid` is true.
3. Keep `Result.Seed`. A seed that fails moves on to the next seeds, so `Result.Seed` can differ from the seed that you asked for.
4. Use the same settings and `Result.Seed` to make the same floor again.

### See each step of a generation

1. Call `Generate(Settings, Seed, true)`, or set **Keep Steps** on `Generate Dungeon`.
2. Read `Result.Steps`. Each step has a name, a note, and a copy of the graph.

### Build a floor from the layout

1. Read `Result.Layout`.
2. For each cell of `Cells`, get the kind: `Empty`, `Room`, `Corridor`, or `Door`.
3. For each entry of `Rooms`, read the node, the first cell (`Min`), and the size.
4. For each entry of `Doors`, read the cell, the edge, the kind, and the direction.
5. Place each cell with `Cell To Local` and your cell size, then the transform of your floor actor.

One cell is one step of the walk grid of your game. Cell (0, 0) has its middle at half a cell from the origin.

### Find the door between two rooms

1. Call `Graph.FindEdge(NodeA, NodeB)`.
2. Read `Graph.Edges[Edge].Kind`.

### Find what a key opens

1. Read `Graph.Keys[Key].LockId`.
2. Read `Graph.Locks[LockId].Edge`.

### Tag a room, for example a camp

1. Open the rule set.
2. Add an entry to **Room Tags**.
3. Set **Tag** to `Camp`, **Count** to 1, and **Place** to a place: `Any`, `DeadEnd`, `MajorCycle`, `NearGoal`, or `Far`.
4. After a generation, look for the tag in `Node.Tags`.

### Know how deep a room is

1. Read `Node.Distance`: the number of doors from the start, through every door.
2. Read `Node.bDeadEnd` for a room with one door.
3. Read `Node.bMinorCycle` for a room on a side loop.

### Check a hand-made graph

1. Make an `FNsDungeonGraph` with `Reset`, `Connect`, `AddLock`, and `AddKey`.
2. Set `StartNode` and `GoalNode`.
3. Call `FNsDungeonGenerator::Validate(Graph, Message)`.
4. If the result is false, read `Message` for the reason.

### Show the overlay on your floor

1. Add an `NsDungeonDebugComponent` to your floor actor.
2. Call `Set Dungeon` with the result and your cell size in cm.
3. In the console, type `NsDungeons.ShowDebug 1`.

Cell (0, 0) starts at the actor origin.

### Leave the demo out of a shipped game

1. Open Project Settings > Packaging.
2. Add the plugin `Demo` folder to **Directories to never cook**.

## Settings

### Rule set (`NsDungeonRuleSet`)

| Setting | Default | Effect |
|---|---|---|
| **Grid Size** | 5 by 5 | The node grid (3 to 16 on each side). Each used node is a room |
| **Min Start Goal Distance** | 3 | The smallest grid distance from the start to the goal (2 or more) |
| **Major Cycles** | LockAndKey 2, TwoKeys 1, OneWayValve 1, Simple 1 | The major cycle types and their weights. A weight of 0 turns a type off |
| **Minor Cycles** | 1 | The side loops after the major cycle |
| **Minor Lock Chance** | 0.5 | The chance of a locked side loop: a lock at its entry, a one-way door out, and the key in an earlier room |
| **Hidden Shortcut Chance** | 0.5 | The chance of a hidden door between two rooms side by side that are far apart on the way |
| **Target Rooms** | 12 | Dead ends grow the floor to this many rooms, as far as the grid allows |
| **Room Tags** | None | Tags to put on rooms, each with a **Tag**, a **Count** (default 1), and a **Place** (default `Any`) |
| **Room Cells Min** | 3 | The smallest room, in cells on each side |
| **Room Cells Max** | 5 | The largest room, in cells on each side. Each room takes a random size between the two |
| **Corridor Cells** | 2 | The cells between two blocks of rooms: the shortest corridor |
| **Lock Kind** | `Door` | The name on new locks |
| **Key Kind** | `Key` | The name on new keys |
| **Max Seed Tries** | 20 | The tries with the next seeds before a failure |

Tag places:

| Place | Rooms |
|---|---|
| `Any` | Any room but the start and the goal |
| `DeadEnd` | A room with one door |
| `MajorCycle` | A room on the major cycle |
| `NearGoal` | The room nearest to the goal, then the next ones |
| `Far` | The room farthest from the start, then the next ones |

### Major cycle types

| Type | Shape |
|---|---|
| `Simple` | Two routes to the goal, no lock |
| `LockAndKey` | The goal is behind a lock. Its key is at the end of the other route, which a one-way door closes off from the goal |
| `TwoKeys` | As `LockAndKey`, with a second key halfway along the locked route |
| `OneWayValve` | One route to the goal. The other route is a one-way way back from the goal |

### Demo actor (`NsDungeonDemoActor`)

| Setting | Default | Effect |
|---|---|---|
| **Rule Set** | None | The rule set. None uses the default settings |
| **Seed** | 7 | The seed of the dungeon |
| **Cell Size** | 100 | The size of a cell in cm |
| **Wall Height** | 150 | The height of the walls in cm |

A change to **Seed** or **Rule Set** in the Details panel builds the floor again in the editor viewport. The **Rebuild** button does the same.

## Debug

- **NsDungeons Preview**: Open Tools > NsDungeons Preview. Pick a rule set (none uses the default settings) and a seed. Then click **Generate**, **Next seed**, or **Random seed**. The **Step** slider moves through the steps. The graph of the step shows on the left, and the cell layout on the right. The text shows the step, the validation result, and the seed that worked. The legend under the views gives the colors.
- **`NsDungeons.SavePreview <Seed> <File.png> [Step] [CycleType]`**: An editor console command. It saves a picture of the preview for a seed. With no file, it saves to `Saved/NsDungeonsPreview.png` of the project. The step starts at 1 (the last step when you leave it out). A cycle type such as `LockAndKey` shows that major cycle alone, with no side loop and no shortcut.
- **`NsDungeons.ShowDebug`**: A console variable, 0 by default. Set it to 1 to draw the overlay of every `NsDungeonDebugComponent` that has a dungeon. The overlay shows the rooms, the doors in their kind colors, and labels for the start, the goal, the tags, the keys, and the locks. The overlay uses debug draw, so a shipping build does not show it. The demo actor sets the variable to 1 during play and gives back the old value at the end.
- **Log category `LogNsDungeons`**: A seed that fails logs the seed that worked. A failure of every try logs an error with the reason.

Colors in the preview and the overlay: green start, gold goal, grey open door, red locked door, orange one-way door, purple hidden door.

## Troubleshooting

| Problem | Cause | Fix |
|---|---|---|
| `bValid` is false, and the log shows "No valid dungeon from seed ..." | No try out of **Max Seed Tries** gave a valid dungeon. A small grid or a large **Min Start Goal Distance** are the usual causes | Make **Grid Size** larger, make **Min Start Goal Distance** smaller, or raise **Max Seed Tries** |
| `Result.Seed` differs from the asked seed | The asked seed failed, and a next seed worked | Keep `Result.Seed` to make the same floor again |
| The floor has fewer rooms than **Target Rooms** | No free grid node is next to a room | Make **Grid Size** larger |
| A room tag is missing | Each room takes one tag, and no free room was left | Lower the tag **Count**, or raise **Target Rooms** |
| The overlay draws nothing | `NsDungeons.ShowDebug` is 0, or `Set Dungeon` was not called, or the build is a shipping build | Set the variable to 1 and call `Set Dungeon` |
| The layout is empty | The result is not valid. Only a valid result gets a layout | Read `Result.Message` and fix the settings |

## Limits

- Corridors are straight, and rooms are in the middle of their block.
- One tag for each room. A tag with no free room of its place goes on any free room.
- Only grid neighbors connect. A room has at most 4 doors.
- The walk grid, the doors that open, and the paths are game code.
- The modules build for Win64 only.
