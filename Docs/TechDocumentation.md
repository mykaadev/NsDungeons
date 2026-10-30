# NsDungeons Technical Documentation

## Architecture

### Modules

| Module | Type | Loading phase | Platforms | Content |
|---|---|---|---|---|
| `NsDungeons` | Runtime | Default | Win64 | The graph, the rule set, the generator, the validation, the layout, the Blueprint library, the debug component, the demo actor |
| `NsDungeonsEditor` | Editor | Default | Win64 | The NsDungeons Preview tab and the `NsDungeons.SavePreview` console command |

The plugin can contain content. `Content/Demo/L_NsDungeonsDemo.umap` is the demo map (`/NsDungeons/Demo/L_NsDungeonsDemo`).

### Folders

| Folder | Content |
|---|---|
| `Source/NsDungeons/Public` | `NsDungeons.h` (module and log category), `NsDungeonGraph.h`, `NsDungeonRuleSet.h`, `NsDungeonGenerator.h`, `NsDungeonLayout.h`, `NsDungeonLibrary.h`, `NsDungeonDebugComponent.h`, `NsDungeonDemoActor.h` |
| `Source/NsDungeons/Private` | The implementations. `NsDungeonGenerator.cpp` holds the main steps and the validation, `NsDungeonSteps.cpp` the later steps, and `NsDungeonLayoutBuilder.cpp` the layout |
| `Source/NsDungeonsEditor/Private` | `NsDungeonsEditorModule.cpp`, `SNsDungeonPreview` (the controls), `SNsDungeonView` (paints a graph step or the layout) |
| `Scripts` | `make_demo_map.py`, which makes the demo map |

### Main Classes

| Type | Responsibility |
|---|---|
| `FNsDungeonGraph` | A fixed grid of `FNsDungeonNode`, a list of `FNsDungeonEdge` between grid neighbors, and the `FNsDungeonLock` and `FNsDungeonKey` lists. Node index = `Y * Size.X + X`. Nodes never move. The steps only mark them |
| `FNsDungeonEdge` | Two nodes, a kind (`None`, `Open`, `Locked`, `OneWay`, `Hidden`), the lock id of a locked edge, and the node that a one-way edge opens from |
| `FNsDungeonLock` | A locked edge and every key id that it needs. A lock with no key never opens |
| `FNsDungeonKey` | The node that holds the key, and its lock |
| `FNsDungeonSettings` | The values of one generation. `UNsDungeonRuleSet` (a `UPrimaryDataAsset`) holds them in an asset |
| `FNsDungeonGenerator` | A struct of static functions: `Generate`, `BuildLayout`, `Validate`, and the private steps |
| `FNsDungeonResult` | The seed that worked, the validity, a message, the graph, the layout, and the optional steps |
| `FNsDungeonLayout` | The cells, the rooms (`FNsDungeonLayoutRoom`), and the doors (`FNsDungeonLayoutDoor`) |
| `UNsDungeonLibrary` | Blueprint calls: `GenerateDungeon`, `GetLayoutCell`, `CellToLocal`, `LocalToCell` |
| `UNsDungeonDebugComponent` | Draws the layout with debug lines and strings each frame while `NsDungeons.ShowDebug` is not 0. It keeps a copy of the result |
| `ANsDungeonDemoActor` | Builds a greybox floor of cube components from a rule set and a seed, binds the demo keys, and floats a step graph over the floor |

### Data Flow

`FNsDungeonGenerator::Generate` runs on the calling thread. For each try, it makes a new result, seeds `FRandomStream(Seed + Try)`, and calls `TryGenerate`. The first try that passes `Validate` gets a layout from `BuildLayout` and returns. After `MaxSeedTries` failures, it logs an error and returns the last result with `bValid` false and no layout.

`TryGenerate` runs these steps in order. With `bKeepSteps`, `KeepStep` stores a copy of the graph in `Result.Steps` after each one, so a valid result has 8 steps.

1. **Grid** (`Reset`). Every node is `Unused`. The size is at least 2 on each side.
2. **StartAndGoal** (`PlaceStartAndGoal`). A random start, and a random goal at `MinStartGoalDistance` or more (grid distance).
3. **MajorCycle** (`DrawMajorCycle`). `FindGridPath` is Dijkstra with a random cost from 1 to 6 on each node, so the path wanders. Arc 0 goes from the start to the goal. Arc 1 does the same around the inner nodes of arc 0. A direct step from the start to the goal is not allowed, so each arc has a node between them. The step tries 8 times. The arcs become `Path` nodes and `Open` edges.
4. **CycleType** (`PickCycleType`, `ApplyCycleType`). A pick by weight. `LockAndKey` and `TwoKeys` lock the last edge of arc 0 into the goal and mark the goal with `LockedArea`. `LockAndKey` puts the key on the last node of arc 1. `TwoKeys` adds a second key halfway along arc 0. All types but `Simple` make the last edge of arc 1 a one-way edge from the goal.
5. **MinorCycles** (`AddMinorCycles`, `NsDungeonSteps.cpp`). From a room, through 1 to 3 unused nodes, back into another room. Up to 20 tries for each loop. By chance, a lock at the entry, a one-way door out, and the key in an earlier room. Then `Validate` runs. If it fails, the step takes the lock, the key, and the one-way door back off.
6. **HiddenShortcut** (`AddHiddenShortcut`). By chance, a `Hidden` edge between the two grid neighbors with the largest gap in doors from the start (more than 2).
7. **DeadEnds** (`AddDeadEnds`). New rooms next to rooms (not the goal) until `TargetRooms`, or until no room has a free neighbor.
8. **Resolve** (`Resolve`). `Distance` from the start, `bDeadEnd`, and the room tags by place, one tag for each room.

A lock marks the rooms behind it with `LockedArea` (the goal for a major lock, the side loop for a minor lock). Steps 5 and 6 never start a side loop or a hidden door from such a room, and never use the goal. Thus they cannot make a way around a lock.

[![The graph of seed 7 after the last step of a generation](Showcase/img/step8.jpg)](Showcase/GenerationSteps.html)

`Validate` walks from the start over the edges that `CanPass` allows. Each reached key goes into the held set, and the walk runs again until nothing changes. The graph is valid when the goal and every key are reached, and each lock matters: with its keys held back, some room is out of reach. A lock that a second way goes around fails the seed.

`BuildLayout` uses its own random stream, from a hash of the seed and the text `NsDungeonsLayout`. Each node owns a square block of `Max(RoomCellsMax, RoomCellsMin)` cells, with `CorridorCells` of rock between blocks. A room takes a random size and sits in the middle of its block, so the middle row and the middle column of a block cross its room. The corridor of an edge runs straight along that line from room A to room B. Its middle cell is the door, with the kind of the edge and the direction from A to B.

## Lifetime

- `FNsDungeonGraph`, `FNsDungeonLayout`, and `FNsDungeonResult` are plain value types. The caller owns them.
- `UNsDungeonRuleSet` is an asset. The engine loads it with the object that refers to it.
- `UNsDungeonDebugComponent` copies the result in `SetDungeon` and draws it in `TickComponent`. The component ticks each frame, also when the variable is 0.
- `ANsDungeonDemoActor` calls `Rebuild` in `OnConstruction`, so the floor shows in the editor viewport. `BeginPlay` builds again only when no layout exists. It then sets the view target of player 0, shows the mouse cursor, sets `NsDungeons.ShowDebug` to 1, and binds the keys. `EndPlay` gives back the old value of `NsDungeons.ShowDebug`. Each rebuild destroys the cube components with the tag `NsDungeonsDemoPiece` and makes new ones.
- `FNsDungeonsEditorModule::StartupModule` registers the preview tab spawner, the Tools menu entry, and the `NsDungeons.SavePreview` command. `ShutdownModule` removes them.

## Extension Points

`FNsDungeonGenerator` has no virtual functions, and its steps are private static functions. To change the generation, change the plugin code:

- **A new major cycle type**: Add a value to `ENsDungeonCycleType` and a case in `ApplyCycleType`. `Validate` checks the new type with no other change.
- **A new step**: Declare a private static function in `FNsDungeonGenerator`, define it in `NsDungeonSteps.cpp`, and call it from `TryGenerate` with a `KeepStep` after it.
- **A new tag place**: Add a value to `ENsDungeonTagPlace` and a case in the filter of `Resolve`.

Without a change to the plugin:

- **A floor builder**: Read `FNsDungeonResult::Layout` in game code. The plugin has no floor builder interface.
- **Lock and key names**: `LockKind` and `KeyKind` are `FName` values that the game maps to its own assets.
- **Debug draw**: `UNsDungeonDebugComponent` is `BlueprintSpawnableComponent`. Add it to any actor and call `SetDungeon(const FNsDungeonResult& InDungeon, float InCellSize = 100.0f)`.

## Dependencies

| Dependency | Kind | Why |
|---|---|---|
| Core, CoreUObject, Engine | Public, `NsDungeons` | Types, reflection, actors, components, data assets |
| InputCore | Private, `NsDungeons` | The demo actor binds keys |
| Core, CoreUObject, Engine | Public, `NsDungeonsEditor` | Base engine types |
| NsDungeons | Private, `NsDungeonsEditor` | The generator that the preview calls |
| InputCore, Slate, SlateCore, UMG, PropertyEditor, ToolMenus, RenderCore, UnrealEd | Private, `NsDungeonsEditor` | The preview tab, the rule set picker, the Tools menu entry, and the off screen render of `NsDungeons.SavePreview` |

The `.uplugin` lists no plugin dependencies.

## Performance

- `Generate` is synchronous. It runs up to `MaxSeedTries` tries, each with all 8 steps.
- `GetEdgesOf` and `FindEdge` scan the whole edge list. `FindGridPath` is an O(n²) Dijkstra over the grid nodes. The grid is at most 16 by 16.
- `AddMinorCycles` calls `Validate` once for each locked side loop.
- `bKeepSteps` copies the whole graph 8 times.
- `UNsDungeonDebugComponent` ticks each frame. It draws only when the variable is not 0 and `ENABLE_DRAW_DEBUG` is set.
- The demo actor makes one static mesh component for each room, corridor cell, wall run, and door piece, and one dynamic material for each color.

## Networking

The plugin does not replicate. The generator uses only `FRandomStream` values from the seed, so the same settings and the same seed give the same dungeon.
