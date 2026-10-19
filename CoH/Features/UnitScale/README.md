# UnitScale

**Flag** `UnitScale` · **Task** [19](../../Docs/Tasks/19-UnitScale.md) · **Public header** [UnitScale.h](UnitScale.h)

CoH proportions. In Zero Hour a soldier is almost half a tank long; in Company of Heroes a tank is several soldiers long. With UnitScale on, vehicles are drawn bigger and infantry a little smaller, and the size the game logic uses follows the model, so shots hit the hull, tanks don't overlap and the selection box fits.

## How it plays

- **Sizes on screen, against retail** (`FACTORS` at the top of [Data/make_pack.py](Data/make_pack.py), the `coh` preset):

  | Class | Factor | Which units |
  |---|---|---|
  | Tanks | ×1.25 | tracked, armoured vehicles (`W3DTankDraw`, `W3DOverlordTankDraw`, the Avenger), the Overlord included |
  | Artillery | ×1.25 | Tomahawk, Inferno Cannon, Nuke Cannon, SCUD Launcher |
  | Light vehicles | ×1.15 | everything else on the ground: Humvee, Technical, Troop Crawler, buggies, dozers, supply trucks, drones |
  | Helicopters | ×1.2 | Comanche, Chinook, Helix |
  | Jets | ×1.0 | unchanged: their airfield pads, and GeneralsArsenal's sortie copies, are sized for them |
  | Infantry | ×0.85 | every soldier, heroes included |

  A soldier goes from about 0.43 of a Crusader's length to about 0.29, near CoH's 0.3. Every general's version of a unit (`AirF_`, `Lazr_`, `Boss_`, ...) gets its class's factor.
- **What follows a unit:** its hulk, and the add-ons that ride on it (the Overlord's and Helix's gattling cannon, bunker and speaker tower), take the unit's factor, so a wreck isn't smaller than the tank was. Debris pieces flung on death keep their retail size.
- **Geometry:** a vehicle's `GeometryMajorRadius`, `GeometryMinorRadius` and `GeometryHeight` grow by its factor. Infantry keep their retail radius (`INFANTRY_KEEPS_RADIUS`), so their spacing, footprint and target size don't change; only their height follows the smaller model. Decal shadows (infantry) shrink with the model; volume shadows follow the mesh by themselves.
- **What the bigger geometry changes** (`coh` preset):
  - **Pathfinding:** most tanks, artillery, dozers and supply trucks need a 4-cell gap instead of 3 (Crusader, Paladin, Battlemaster, Scorpion, Tomahawk, Inferno Cannon, SCUD); the Marauder, Battle Bus, Bomb Truck and Listening Outpost 5 instead of 4, the most there is (the Overlord, already 5, stays 5). `make_pack.py` prints the full list. Narrow gaps between buildings, cliffs and walls that a tank used to squeeze through may be closed to it now; bridges and the maps' passes are wide enough for the Overlord, which already needed 5.
  - **Weapon reach:** ranges are measured between the edges of the units' bounding circles (`FROM_BOUNDINGSPHERE_2D` in `Weapon.cpp`), so a tank reaches a tank about 9 units sooner (+5% on a 150 range) and a soldier about 4.5. Infantry against infantry is unchanged. Weapon ranges are left as they are.
  - **Accuracy:** [Accuracy](../Accuracy/README.md)'s target size reads the bounding radius, so a scaled tank is a little easier to hit (a Crusader ×0.95 → ×1.04, a Humvee ×0.89 → ×0.95, an Overlord ×1.14 → ×1.26). Soldiers keep their radius and their ×0.6 to ×0.7.
  - **Crushing and collisions:** a bigger tank crushes and bumps over a wider strip; the engine's crusher rules are unchanged.
  - **Splash:** blast radii are not scaled. A blast centred on a bigger hull reaches a little less far beyond it.
  - **Cover and Directional armor** (task 05 and 07, not merged when this was written) read the geometry too: Cover's shapes (`Shape::box` from the major and minor radius) and its blast origin (the victim's bounding radius), Directional armor's hit side. With UnitScale on they see the scaled hulls and hulks: a bigger tank or wreck gives more cover. Check their tuning with both flags on.
- Everything is INI: the flag off loads no pack, and every unit is retail.

## Choosing the factors

`python CoH/Features/UnitScale/Data/make_pack.py --preset <name> --out <folder>` writes one of the presets elsewhere, to try it in a test folder without touching the slice's pack:

| Preset | Tanks, artillery | Light | Helicopters | Infantry | A soldier against a Crusader's length |
|---|---|---|---|---|---|
| `retail` | ×1.0 | ×1.0 | ×1.0 | ×1.0 | 0.43 (scales nothing; the showcase's retail picture) |
| `subtle` | ×1.15 | ×1.10 | ×1.15 | ×0.90 | 0.34 |
| `coh` | ×1.25 | ×1.15 | ×1.20 | ×0.85 | 0.29 (the pack) |
| `big` | ×1.40, ×1.35 | ×1.25 | ×1.30 | ×0.80 | 0.25 |

`-cohUnitScaleShowcase` (with the flag on) lines up each faction's soldiers and vehicles in front of the local player's command center at the start of a new game, and points the camera at them, for screenshots:

```powershell
generalszh.exe -cohSkirmish "Tournament Desert" -coh UnitScale -cohSide FactionAmerica -win -xres 1600 -yres 900 -cohUnitScaleShowcase
```

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
| `Data/INI/10_UnitScale.ini` | the pack (generated): `Scale`, geometry and decal shadow size for 364 objects |

## Testing

- Test folder: `C:\Users\ibm21\Games\ZH-UnitScale` (retail files hardlinked; `install.ps1 -GameDir`).
- Replays: `Replays\CoHUnitScale\`, recorded with the `coh` pack; any change to the factors makes them mismatch, so re-record them.

## Engine notes

- **`Scale` is applied twice.** An object's `Scale` (ThingTemplate `m_assetScale`, the Drawable's instance scale) scales the model when it's made (`W3DAssetManager::Create_Render_Obj` → `HLodClass::Scale`: mesh vertices and bone offsets) and again in the drawn transform (`W3DModelDraw::doDrawModule`). So `Scale = s` draws the model at s²; the pack writes `Scale = factor^½`. Measured: a dozer at `Scale = 2` drew four times as long.
- The bones the logic fires from and positions add-ons at (`W3DModelDraw::getPristineBonePositionsForConditionState`, `getProjectileLaunchOffset`, through `ModelConditionInfo::validateStuff`) are measured the same way, scaled model and scaled transform, so muzzles and turrets stay on the model. That is logic: a pack's `Scale` changes where shots start, deterministically.
- Geometry fields override one at a time in a pack (`GeometryInfo::parseGeometryMajorRadius` and the others each recompute the bounding values), so a pack writes only the fields it changes.
- `ObjectReskin` and `ChildObject` copy their base when they are parsed; a pack that changes the base doesn't reach them, so every general's copy gets its own block.
- The pathfinding footprint is `Pathfinder::getRadiusAndCenter` (`AIPathfind.cpp`): from the bounding circle's diameter in 10-unit cells, a diameter between 10 and 20 counted as 20, at most 5 cells.
- Retail INI has malformed numbers (`GeometryHeight = 40v.0`); the engine reads the leading number, and `make_pack.py` does too.
