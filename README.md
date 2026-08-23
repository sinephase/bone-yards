# Bone Yards: Derelict Frigate Archaeology

A sci-fi action game built on the **Yamagi Quake II** engine, featuring in-game editing, pixelated aesthetics, and power-driven gameplay.

## Core Concept

You are a field archaeologist-technician of the **3rd Regiment of Eron**, sent into the **bone yards**—vast orbital graveyards of ancient derelict frigates. These ships are held in semi-shutdown state with barely-functional fusion reactors that leak power across millennia.

Over ages, this residual radiation has attracted and mutated life:
- **Mechalichen**: Cybernetic fungal colonies that hijack ship systems
- **Nuclear Slugs**: Soft-bodied radiotrophs that grow stronger as power levels rise

Your mission:
1. Reach the **command deck** and restore access to the ship's logs and manifests
2. Decrypt the data to gain **engineering access codes**
3. Navigate the **engineering decks** while managing rising power and creature aggression
4. Confront the apex organism nested around the primary fusion plant
5. Extract the ship's deepest secrets (schematics, lost histories)

## Key Features

- **Power-as-Gameplay**: Restoring power opens doors but feeds creatures
- **In-Game Editing**: Runtime level editor with drag & drop
- **Pixelated Aesthetic**: Low-poly models + pixel-art textures
- **Environmental Storytelling**: Cryptic logs, radiation-scarred corridors
- **Dynamic Difficulty**: Creatures escalate as power rises
- **Layered Exploration**: Metroidvania-style progression

## Technical Foundation

Built on **Yamagi Quake II** (64-bit clean, modern OpenGL 3.2 renderer, GPL v2).

**Language**: C/C++ | **Build**: CMake | **Platforms**: Linux, Windows, macOS, FreeBSD

## Project Structure

```
bone-yards/
├── src/
│   └── bone_yards/           # Game-specific code
│       ├── by_local.h
│       ├── by_main.c
│       ├── by_power.c
│       ├── by_player.c
│       ├── by_enemy.c
│       ├── by_editor.c
│       ├── by_hazard.c
│       └── by_spawn.c
├── assets/
│   └── scripts/
│       ├── entities.json
│       └── power_zones.json
├── docs/
│   ���── (to be added)
└── CMakeLists.txt
```

## Building

### Prerequisites

- CMake 3.10+
- C99-compliant compiler (gcc, clang)
- OpenGL 3.2+ support
- JPEG, PNG, zlib development libraries

### Linux/macOS

```bash
git clone https://github.com/sinephase/bone-yards.git
cd bone-yards
mkdir build && cd build
cmake ..
make -j$(nproc)
```

### Windows

```bash
cmake -G "Visual Studio 16 2019" ..
cmake --build . --config Release
```

## Running

```bash
./bone-yards
```

In-game editor accessible via console: `by_editor 1`

## Documentation

- **[DESIGN.md](./DESIGN.md)** — Full game design document, lore, mechanics

## License

This project is built on **Yamagi Quake II**, which is licensed under **GPLv2**. All custom code is also licensed under **GPLv2** per the Quake II source release.

## Credits

- **Yamagi Quake II** — Modern Quake II port and maintenance
- **id Software** — Original Quake II engine and game design
- **Bone Yards Concept** — Custom design for this project
