# Bone Yards: Game Design Document

## High Concept

**Bone Yards** is a single-player sci-fi exploration/action game set in an orbital graveyard of ancient derelict warships. The player (a field archaeologist) must delve into the ruins of a massive frigate to recover lost technology and knowledge, while contending with radiation-feeding creatures and a failing reactor system.

**Core Loop**: Explore → Restore Power → Unlock New Areas → Encounter Escalating Threats → Extract Secrets

---

## Setting & Lore

### The World

The year is **3847 CE**. The **3rd Regiment of Eron** is a military-science faction that explores and salvages ancient derelict spacecraft in orbital boneyards—vast graveyards of ships abandoned millennia ago.

These frigates are held in "safe" semi-shutdown mode, their fusion reactors never fully decommissioned because complete shutdown was deemed more dangerous than controlled leakage. Over aeons, residual radiation and power leakage created conditions for bizarre life forms to evolve.

### The Creatures

**Mechalichen**: Cybernetic fungal colonies. They integrate directly into ship systems, forming networks that span entire sections of the frigate. They hijack doors, lights, turrets, and weapons. They are not mindless—they communicate, coordinate, and learn.

**Nuclear Slugs**: Soft-bodied radiotrophs that feed directly on fusion radiation. They are simple creatures but exponentially more aggressive and powerful when power levels rise. They leave trails of radioactive slime.

**The Boss (Apex Organism)**: An enormous creature that has nested around the primary fusion plant itself. It has grown grotesquely large by decades of exposure to core radiation. Killing it (or forcing it dormant) is the only way to safely access the deepest data vaults.

---

## Core Mechanics

### 1. Power-as-Gameplay

**Central Conceit**: Restoring power is necessary to progress (unlock doors, access terminals, activate lifts) but also **directly feeds the creatures**, escalating difficulty.

**Mechanics**:
- **Power Domains**: The frigate is divided into electrical zones (command, engineering, cargo, etc.)
- **Power Levels** (0–100%):
  - 0–20%: Dormant creatures, minimal hazards
  - 21–50%: Creatures alert, basic attacks
  - 51–80%: Creatures aggressive, coordinated attacks, radiation spikes
  - 81–100%: Creatures frenzy, radiation critical, environmental hazards
- **Player Choice**: You can restore power piecemeal or strategically power down zones after accessing needed data

### 2. Exploration & Progression

Metroidvania-style gating with power-based progression.

### 3. Combat (Tactical, Not Arcadey)

You are not a soldier—you're an archaeologist with limited combat ability. Combat is dangerous. Running and hiding is always an option.

---

## Level Structure

### Act I: Insertion & Outer Hull
### Act II: Command Deck & Data Recovery
### Act III: Engineering Descent
### Act IV: Fusion Core & Final Boss

---

## In-Game Editor

Fully functional runtime editor for:
- **Entity Placement**: Click-to-spawn entities (enemies, items, hazards)
- **Property Editing**: Real-time value tweaking
- **Power Zone Setup**: Define and connect power domains
- **Save/Load**: Serialize level state

---

## Technical Notes

- Built on Yamagi Quake II engine
- C/C++ codebase (64-bit clean)
- Modern OpenGL 3.2 rendering
- CMake build system
- Cross-platform (Linux, Windows, macOS, FreeBSD)
