# UnitScale

**Flag** `UnitScale` · **Task** [19](../../Docs/Tasks/19-UnitScale.md) · **Public header** [UnitScale.h](UnitScale.h)

CoH proportions. In Zero Hour a soldier is almost half a tank long; in Company of Heroes a tank is several soldiers long. With UnitScale on, vehicles are drawn bigger next to soldiers of retail size, and the size the game logic uses follows the model, so shots hit the hull, tanks don't overlap and the selection box fits.

## How it plays

- **Sizes on screen, against retail** (`FACTORS` at the top of [Data/make_pack.py](Data/make_pack.py), the `chosen` preset):

  | Class | Factor | Which units |
  |---|---|---|
  | Tanks | ×1.45 | tracked, armoured vehicles (`W3DTankDraw`, `W3DOverlordTankDraw`, the Avenger), the Overlord included |
  | Artillery | ×1.3 | Tomahawk, Inferno Cannon, Nuke Cannon, SCUD Launcher |
  | Heavy machines | ×1.35 | dozers and supply trucks (`DOZER`, `HARVESTER`): a real bulldozer is about 8 m long, as long as a tank |
  | Light vehicles | ×1.05 | everything else on the ground: Humvee, Technical, Troop Crawler, buggies, drones. A Humvee stays near its real size next to a soldier |
  | Helicopters | ×1.25 | Comanche, Chinook, Helix |
  | Jets | ×1.0 | unchanged: their airfield pads, and GeneralsArsenal's sortie copies, are sized for them |
  | Infantry | ×1.0 | unchanged, so soldiers stay readable at Zero Hour's camera height |

  A soldier goes from about 0.43 of a Crusader's length to about 0.30, near CoH's. Every general's version of a unit (`AirF_`, `Lazr_`, `Boss_`, ...) gets its class's factor.
- **What follows a unit:** its hulk, and the add-ons that ride on it (the Overlord's and Helix's gattling cannon, bunker and speaker tower), take the unit's factor, so a wreck isn't smaller than the tank was. Debris pieces flung on death keep their retail size.
- **Geometry:** a vehicle's `GeometryMajorRadius`, `GeometryMinorRadius` and `GeometryHeight` grow by its factor. Volume shadows follow the mesh by themselves. (A preset that shrinks infantry scales only their height and decal shadow, `INFANTRY_KEEPS_RADIUS`.)
- **What the bigger geometry changes:**
  - **Pathfinding:** most tanks, dozers and supply trucks need a 5-cell gap instead of 3, the most there is, like the Overlord already did (Crusader, Paladin, Scorpion, Gattling, Dragon, ECM); the Battlemaster, Tomahawk, Inferno Cannon and SCUD need 4. `make_pack.py` prints the full list. Narrow gaps between buildings, cliffs and walls that a tank used to squeeze through may be closed to it now; bridges and the maps' passes are wide enough for the Overlord.
  - **Weapon reach:** ranges are measured between the edges of the units' bounding circles (`FROM_BOUNDINGSPHERE_2D` in `Weapon.cpp`), so a tank reaches a tank about 16 units sooner (+11% on a 150 range) and a soldier about 8. Infantry against infantry is unchanged. Weapon ranges are left as they are.
  - **Accuracy:** [Accuracy](../Accuracy/README.md)'s target size reads the bounding radius, so a scaled tank is easier to hit (a Crusader ×0.95 → ×1.09, an Overlord ×1.14 → ×1.35, a Humvee ×0.89 → ×0.91). Soldiers keep their ×0.6 to ×0.7.
  - **Crushing and collisions:** a bigger tank crushes and bumps over a wider strip; the engine's crusher rules are unchanged.
  - **Splash:** blast radii are not scaled. A blast centred on a bigger hull reaches a little less far beyond it.
  - **Cover and Directional armor** (task 05, not merged when this was written, and [07](../DirectionalArmor/README.md)) read the geometry too: Cover's shapes (`Shape::box` from the major and minor radius) and its blast origin (the victim's bounding radius), Directional armor's hit side. With UnitScale on they see the scaled hulls and hulks: a bigger tank or wreck gives more cover. Check their tuning with both flags on.
- Everything is INI: the flag off loads no pack, and every unit is retail.

## Choosing the factors

`python CoH/Features/UnitScale/Data/make_pack.py --preset <name> --out <folder>` writes one of the presets elsewhere, to try it in a test folder without touching the slice's pack:

| Preset | Tanks, artillery | Heavy | Light | Helicopters | Infantry | A soldier against a Crusader's length |
|---|---|---|---|---|---|---|
| `retail` | ×1.0 | ×1.0 | ×1.0 | ×1.0 | ×1.0 | 0.43 (scales nothing; the showcase's retail picture) |
| `subtle` | ×1.15 | ×1.10 | ×1.10 | ×1.15 | ×0.90 | 0.34 |
| `coh` | ×1.25 | ×1.15 | ×1.15 | ×1.20 | ×0.85 | 0.29 |
| `big` | ×1.40, ×1.35 | ×1.25 | ×1.25 | ×1.30 | ×0.80 | 0.25 |
| `chosen` | ×1.45, ×1.30 | ×1.35 | ×1.05 | ×1.25 | ×1.0 | 0.30 (the pack) |

The user compared them in game (2026-10-19): the presets that shrink soldiers made them too small and the Humvee too big next to them; `chosen` keeps soldiers as they are and grows tanks and heavy machines.

`-cohUnitScaleShowcase` (with the flag on) lines up each faction's vehicles, a soldier standing beside each, in front of the local player's command center at the start of a new game, and points the camera at them, for screenshots:

```powershell
generalszh.exe -cohSkirmish "Tournament Desert" -coh UnitScale -cohSide FactionAmerica -win -xres 1600 -yres 900 -cohUnitScaleShowcase
```

Zero Hour's camera (`CameraHeight` 232 in `GameData.ini`) sits much higher than CoH's, which makes everything look small. A `GameData` block in the pack did not change it; a closer camera would be its own task.

## Skirmish AI

Nothing to do: the AI plans with the same geometry the units have. Its base layouts were placed for retail sizes, so a big tank may need to go around a tight corner of its own base.

## Hooks

None. The pack does the work; the code is the flag's `isActive` and the showcase option (hub `gameStartAfterMap`).

## Files

| File | What |
|---|---|
| `UnitScale.h` | `isActive()` |
| `Engine/UnitScale.cpp` | the feature |
| `Engine/Showcase.cpp` | the `-cohUnitScaleShowcase` test option |
| `Data/make_pack.py` | writes the pack from the retail INI; the factors and presets are at its top |
| `Data/INI/10_UnitScale.ini` | the pack (generated): `Scale` and geometry for 238 objects |

## Testing

- Test folders: `C:\Users\ibm21\Games\ZH-UnitScale` and `ZH-UnitScale2` (retail files hardlinked; `install.ps1 -GameDir`).
- Replays: `Replays\CoHUnitScale\`, recorded with the `chosen` pack; any change to the factors makes them mismatch, so re-record them.

## Engine notes

- **`Scale` is applied twice.** An object's `Scale` (ThingTemplate `m_assetScale`, the Drawable's instance scale) scales the model when it's made (`W3DAssetManager::Create_Render_Obj` → `HLodClass::Scale`: mesh vertices and bone offsets) and again in the drawn transform (`W3DModelDraw::doDrawModule`). So `Scale = s` draws the model at s²; the pack writes `Scale = factor^½`. Measured: a dozer at `Scale = 2` drew four times as long.
- The bones the logic fires from and positions add-ons at (`W3DModelDraw::getPristineBonePositionsForConditionState`, `getProjectileLaunchOffset`, through `ModelConditionInfo::validateStuff`) are measured the same way, scaled model and scaled transform, so muzzles and turrets stay on the model. That is logic: a pack's `Scale` changes where shots start, deterministically.
- Those bones are cached in the draw module's data, which an override shares with its base template, at the scale of the first object that asks. Without the `W3DModuleFactory::reset` fix (`CoH/README.md`, *Fixes in original code*), a game with UnitScale left its scaled bones to the next game in the same process, and replays played after it mismatched (`Replays\CoHOverrideLeakRepro`, frame 6900).
- Geometry fields override one at a time in a pack (`GeometryInfo::parseGeometryMajorRadius` and the others each recompute the bounding values), so a pack writes only the fields it changes.
- `ObjectReskin` and `ChildObject` copy their base when they are parsed; a pack that changes the base doesn't reach them, so every general's copy gets its own block.
- The pathfinding footprint is `Pathfinder::getRadiusAndCenter` (`AIPathfind.cpp`): from the bounding circle's diameter in 10-unit cells, a diameter between 10 and 20 counted as 20, at most 5 cells.
- Retail INI has malformed numbers (`GeometryHeight = 40v.0`); the engine reads the leading number, and `make_pack.py` does too.
