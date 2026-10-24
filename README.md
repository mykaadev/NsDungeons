<!-- GH_ONLY_START -->
<h1 align="center">
  <br>
  <a href="https://github.com/mykaadev/NsDungeons"><img src="https://raw.githubusercontent.com/mykaadev/NsDungeons/refs/heads/main/Resources/Banner.png" alt="NsDungeons" width="400"></a>
</h1>

<h4 align="center">Cyclic lock-and-key dungeon generation on a grid for Unreal Engine.</h4>

<div align="center">
    <a href="https://github.com/mykaadev/NsDungeons/commits/main"><img src="https://img.shields.io/github/last-commit/mykaadev/NsDungeons?style=plastic&logo=github&logoColor=white" alt="GitHub Last Commit"></a>
    <a href="https://github.com/mykaadev/NsDungeons/issues"><img src="https://img.shields.io/github/issues-raw/mykaadev/NsDungeons?style=plastic&logo=github&logoColor=white" alt="GitHub Issues"></a>
    <a href="https://github.com/mykaadev/NsDungeons/pulls"><img src="https://img.shields.io/github/issues-pr-raw/mykaadev/NsDungeons?style=plastic&logo=github&logoColor=white" alt="GitHub Pull Requests"></a>
    <a href="https://github.com/mykaadev/NsDungeons"><img src="https://img.shields.io/github/stars/mykaadev/NsDungeons?style=plastic&logo=github" alt="GitHub Stars"></a>
    <a href="https://twitter.com/mykaadev/"><img src="https://img.shields.io/twitter/follow/mykaadev?style=plastic&logo=x" alt="Twitter Follow"></a>

<p style="display:none;">
  <a href="#-summary">👀 Summary</a> •
  <a href="#-features">📦 Features</a> •
  <a href="#-requirements">⚙️ Requirements</a> •
  <a href="#-installation">🛠️ Installation</a> •
  <a href="#-getting-started">🚀 Getting Started</a> •
  <a href="#-major-cycle-types">🌀 Major Cycle Types</a> •
  <a href="#-api">🔧 API</a> •
  <a href="#-limits">🚧 Limits</a> •
  <a href="#-acknowledgement">🧾 Acknowledgement</a> •
  <a href="#-documentation">📚 Documentation</a> •
  <a href="#-credits">❤️ Credits</a> •
  <a href="#-support">📞 Support</a> •
  <a href="#-license">📃 License</a>
</p>
<a href="https://buymeacoffee.com/mykaadev"><img src="https://www.svgrepo.com/show/476855/coffee-to-go.svg" alt="Coffee" width="50px"></a>
<p><b>Buy me a coffee!</b></p>
</div>
<!-- GH_ONLY_END -->

## 👀 Summary
NsDungeons makes dungeon floors with a lock-and-key structure, after the cyclic method of Unexplored. It gives an abstract graph on a fixed grid (rooms, doors, locks, keys, and one-way doors), and a cell layout of that graph. An editor window shows each step of a generation.

NsDungeons does not build game floors, move characters, or find paths. The game builds its floor from the layout.

<div align="center">
 <img src="https://github.com/mykaadev/NsDungeons/blob/main/Docs/Images/Preview.png" alt="NsDungeons Preview" width="750">
</div>

## 📦 Features
- **Seeded generator**: The same settings and the same seed always give the same dungeon.
- **Cycles**: A major cycle of two arcs, with four cycle types, and minor side loops.
- **Locks and keys**: Locks, keys, one-way doors, and hidden shortcuts.
- **Validation**: Every dungeon is solvable, and every lock matters.
- **Cell layout**: Rooms, corridors, and doors on a cell grid, ready for a floor builder.
- **Room tags**: Put game tags such as `Camp` on rooms, by place.
- **Editor preview**: Step through a generation in the NsDungeons Preview tab.
- **Play overlay**: Draw the rooms, doors, keys, and locks over a floor in play.
- **Blueprint support**: Generate a dungeon and read its cells from Blueprint.

## ⚙️ Requirements
Unreal Engine 5.7+

## 🛠️ Installation
1. **Clone** or download this repository.
2. Copy the `NsDungeons` folder into the `Plugins` folder of your project.
3. Add `"NsDungeons"` to the dependencies of the game module that calls the generator.
4. Generate the project files and enable the plugin.
5. Make a rule set asset: Content Browser, Miscellaneous, Data Asset, `NsDungeonRuleSet`.

## 🚀 Getting Started
```cpp
#include "NsDungeonGenerator.h"

const FNsDungeonResult Result = FNsDungeonGenerator::Generate(RuleSet->Settings, Seed);
if (Result.bValid)
{
    for (const FNsDungeonNode& Node : Result.Graph.Nodes)
    {
        // Node.Role, Node.Coord, Node.Tags: one room for each used node
    }

    // Result.Layout: Cells, Rooms, and Doors for the floor builder
}
```

In Blueprint, `Generate Dungeon` (rule set, seed) returns the result with its graph and its layout. `Get Layout Cell`, `Cell To Local`, and `Local To Cell` read the cells.

To see the plugin work, open the map `/NsDungeons/Demo/L_NsDungeonsDemo` and press Play. Use N and P to change the seed, and 1 to 9 to show the graph of a step.

<div align="center">
 <img src="https://github.com/mykaadev/NsDungeons/blob/main/Docs/Images/Demo.png" alt="NsDungeons Demo" width="750">
</div>

## 🌀 Major Cycle Types
| Type | Shape |
|---|---|
| `Simple` | Two routes to the goal, no lock |
| `LockAndKey` | The goal is behind a lock. Its key is at the end of the other route, which a one-way door closes off from the goal |
| `TwoKeys` | As `LockAndKey`, with a second key halfway along the locked route |
| `OneWayValve` | One route to the goal. The other route is a one-way way back from the goal |

## 🔧 API
### Data
- `FNsDungeonGraph` – A fixed grid of nodes, the edges between grid neighbors, and the lock and key lists.
- `FNsDungeonEdge` – Two nodes and a kind: `Open`, `Locked`, `OneWay`, or `Hidden`.
- `FNsDungeonLock` / `FNsDungeonKey` – A locked edge and the keys that it needs, and the node that holds each key.
- `FNsDungeonSettings` / `UNsDungeonRuleSet` – The values of one generation, and the asset that holds them.
- `FNsDungeonResult` – The seed that worked, the validity, the graph, the layout, and the steps.

### Main Functions
- `FNsDungeonGenerator::Generate` – Make a dungeon. Pass `true` as the third argument to keep each step.
- `FNsDungeonGenerator::Validate` – Check a hand-made graph.
- `UNsDungeonLibrary` – Blueprint calls: generate, read a cell, cell to local, and local to cell.
- `UNsDungeonDebugComponent` – The play overlay. Call `SetDungeon`, then set `NsDungeons.ShowDebug 1`.

## 🚧 Limits
- Corridors are straight, and rooms are in the middle of their block.
- One tag for each room.
- Only grid neighbors connect, so a room has at most 4 doors.

## 🧾 Acknowledgement
The generation follows the cyclic dungeon method of **Unexplored**, as Boris the Brave describes it (2021).

## 📚 Documentation
- [User Guide](https://mykaadev.github.io/plugins/nsdungeons/user-guide/) – Setup, common tasks, settings, and debug tools.
- [Technical Documentation](https://mykaadev.github.io/plugins/nsdungeons/tech-documentation/) – Architecture, lifetime, extension points, and tests.

<!-- GH_ONLY_START -->
## ❤️ Credits
<a href="https://github.com/mykaadev/NsDungeons/graphs/contributors"><img src="https://contrib.rocks/image?repo=mykaadev/NsDungeons"/></a>

## 📞 Support
Reach out via the **[profile page](https://github.com/mykaadev)**.

## 📃 License
[![License](https://img.shields.io/badge/license-MIT-green)](https://www.tldrlegal.com/license/mit-license)
<!-- GH_ONLY_END -->
