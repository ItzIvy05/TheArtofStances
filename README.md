# Stances CommonLib XP Base

Native SKSE/CommonLibSSE NG base for **The Art of Stances** custom skill XP.

## What it does

- Registers an `ActorKill` event sink.
- Awards Custom Skills Framework XP when the player kills an actor.
- Calls `AdvanceSkill("TheArtofStance", xp)` instead of manually editing level globals.
- Lets Custom Skills Framework use `Stances.json` for vanilla-style leveling, ratio, legendary, and perk points.
- Includes a simple INI at `Data/SKSE/Plugins/Stances.ini`.

## Current skill id

Your uploaded custom skill tree uses:

```json
"id": "TheArtofStance"
```

The DLL defaults to that id.

## Build notes

Set `COMMONLIB_SSE_FOLDER` to your CommonLibSSE NG folder, then build with the existing CMake presets.

```bat
cmake --preset release
cmake --build build/release --config Release
```

This is only the XP base. Stance-specific checks can be added later once the stance state/source is finalized.
