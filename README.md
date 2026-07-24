# The Art of Stances

Native SKSE/CommonLibSSE-NG plugin powering **The Art of Stances**: a custom stance system with its own
Custom Skills Framework skill tree, kill-based leveling, and a Scaleform HUD widget.

## Features

- **Three stances** Bear, Wolf, Hawk — switched by hotkey or a dedicated cycle key. Neutral clears the stance.
- **Perk-gated**: a stance can only be entered once its tier-1 "Aspects of the ..." perk is taken.
  Taking that perk auto-applies the stance. Cycling skips locked stances.
- **Custom skill tree** via [Custom Skills Framework](https://www.nexusmods.com/skyrimspecialedition/mods/41780):
  kills grant skill XP (`fSkillUsePerKill` + victim-level scaling), which levels the Stances skill and
  earns perk points for the three perk lines (4 perks each at skill 0/25/50/75).
- **HUD widget**: an A/B/C indicator (A = Bear, B = Wolf, C = Hawk) pops up on stance changes and hides
  after a configurable delay (3-15 s). Position/scale configurable.
- **Savage Instinct** (Wolf 75) is handled natively by the DLL: killing an enemy in Wolf stance casts the
  weapon-damage buff. No Papyrus.
- **In-game settings** via [SKSE Menu Framework](https://www.nexusmods.com/skyrimspecialedition/mods/120352)
  (optional): click-to-capture hotkey rebinding, HUD position/scale/duration sliders. Without SMF, everything
  is configurable in `Data/SKSE/Plugins/Stances.ini`.

## Runtime layout

- `The Art of Stances.esp` (ESL): stance abilities/effects, Aspect perks, skill globals.
- `SKSE/Plugins/Stances.dll` + `Stances.ini`
- `SKSE/Plugins/CustomSkills/` skill definition (`SKILLS.json`, `Stances/Stances.json`)
- `Interface/exported/widgets/StancesHUD.swf` (HUD widget)
- `Interface/Translations/The Art of Stances_ENGLISH.txt`

Stance state for other mods (OAR conditions etc.): `HasMagicEffect` on
`The Art of Stances.esp|903` (Bear), `|904` (Wolf), `|905` (Hawk); current/previous stance globals at
`|917` / `|916` (0 = Neutral, 1 = Bear, 2 = Wolf, 3 = Hawk).

## Build

Requires `COMMONLIB_SSE_FOLDER` pointing at CommonLibSSE-NG and `VCPKG_ROOT`. MSVC toolset 14.44
(the vcpkg overlay triplet in `cmake/` pins it).

```bat
cmake --preset release
cmake --build build/release
```
