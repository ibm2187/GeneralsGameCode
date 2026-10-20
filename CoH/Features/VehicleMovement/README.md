# VehicleMovement

Task [20](../../Docs/Tasks/20-VehicleMovement.md) · Flag `VehicleMovement` · Public header [`VehicleMovement.h`](VehicleMovement.h)

Vehicles drive like what they are: a heavy tank is slow to get going, slow to stop and slow to turn, a buggy darts about, and armor plates and riders make a tank heavier. Trucks are quickest on roads; rock, sand and snow slow everything, wheels most.

## Behavior (Phase 1: weight)

- **Weight:** every ground vehicle weighs some tons: its `Unit` line's class (`Light` 6, `Medium` 30, `Heavy` 45, `SuperHeavy` 70) or its own tons. A vehicle no line names weighs `DefaultClass` (Medium). Helicopters, jets, hover vehicles and infantry have none and drive as in retail.
- **Upgrades add weight** (`Upgrade` lines): Composite Armor +4, the Overlord's gattling +3, speaker tower +3, bunker +6, the Scorpion's rocket +1. An object upgrade counts once the vehicle has it, bought or fitted by Tank loadouts (18), which gives the same upgrades. A player upgrade counts on each vehicle one of whose upgrade modules answers to it, so Composite Armor weighs on Crusaders and Paladins but not Humvees. The weight is worked out again whenever the vehicle's or its owner's upgrades change.
- **Handling:** the weight reads five factors off the pack's curves (straight lines between points), on the vehicle's retail values:

  | | 0.5 t (bike) | 6 t (Light) | 30 t (Medium) | 45 t (Heavy) | 70 t (Overlord) | 80 t (Emperor) |
  |---|---|---|---|---|---|---|
  | Top speed | 120% | 112% | 94% | 88% | 78% | 74% |
  | Acceleration | 170% | 150% | 100% | 80% | 55% | 50% |
  | Braking | 160% | 140% | 100% | 85% | 65% | 60% |
  | Turn rate | 130% | 125% | 100% | 88% | 72% | 68% |
  | Speed lost on a 100% grade | 20% | 40% | 90% | 110% | 150% | 160% |

- **Hills:** going uphill, a vehicle loses its weight's share of speed per grade (rise over distance, measured `SlopeLookAhead` ahead of its nose), never below `SlopeMinSpeed` (40%). Downhill and flat ground keep full speed. Retail ground vehicles have no slope slowdown at all, so even a Medium is a little slower uphill.
- **Ground and roads** (asked for after the first watch test): top speed also depends on what a vehicle drives on, and on its drive. Wheels (four wheels, motorcycles) gain more on roads and lose more off them; treads hardly care:

  | Surface | Wheels | Treads | From |
  |---|---|---|---|
  | Paved | 120% | 105% | two- and four-lane roads, asphalt, concrete and parking tiles, urban ground, bridges |
  | Track | 110% | 102% | dirt, gravel, sand and cobblestone roads, cobble tiles |
  | Open | 100% | 100% | everything else, desert sand included (deserts are made of it) |
  | Sand | 85% | 95% | tropical beaches |
  | Rock | 80% | 90% | rock and cliff tiles, rugged mountains, rock accents |
  | Snow | 90% | 95% | flat snow, dirty snow |
  | DeepSnow | 75% | 85% | rugged snow |

  Pathfinding doesn't know about it: units don't go out of their way for a road, they're just quicker where their path runs along one.
- The skirmish AI's vehicles follow the same rules.

## Speed order

All factors multiply, so the order doesn't change the result:
1. the locomotor's retail speed (or its damaged speed) and the engine's caps (`Locomotor::setMaxSpeed`, a group's speed);
2. hub `unitSpeed`: Veterancy's rank speed, then this slice's weight, slope and ground (feature order in `CoH/CMakeLists.txt`);
3. Retreat's (11) retreat bonus, on its own hub point after `getMaxSpeedForCondition`, once it is merged.

Pacing changes only prices and build times, UnitScale only sizes; neither touches movement.

## INI

The `CoHVehicleMovement` block in `Data/INI/10_VehicleMovement.ini`:

| Line | Example | What |
|---|---|---|
| `Class` | `Class = Heavy 45` | a weight class and its tons |
| `DefaultClass` | `Medium` | the class of vehicles no `Unit` line names |
| `Unit` | `Unit = ChinaTankOverlord SuperHeavy`, `Unit = GLAVehicleCombatBike 0.5` | a vehicle's class or tons; covers the generals' versions (`Nuke_ChinaTankOverlord`), which may have lines of their own; names match without case |
| `Upgrade` | `Upgrade = Upgrade_AmericaCompositeArmor 4` | the tons an upgrade adds |
| `Speed`, `Acceleration`, `Braking`, `TurnRate` | `Speed = 6 112% 30 100% 70 82%` | a curve: `<tons> <percent>` pairs |
| `SlopePenalty` | `SlopePenalty = 30 90% 70 150%` | a curve: the share of speed lost on a 100% grade |
| `SlopeMinSpeed` | `40%` | a hill never slows below this |
| `SlopeLookAhead` | `20.0` | how far ahead of the vehicle the grade is measured |
| `Vehicles`, `Exempt` | `VEHICLE`, `AIRCRAFT` | what has a weight: these kinds but not the exempt ones |
| `Surface` | `Surface = Paved 120% 105%` | a kind of ground: top speed on it for wheels and for treads |
| `Tile` | `Tile = Asphalt Paved` | the surface of terrain textures whose `Terrain.ini` name starts so; wins over `Ground` |
| `Ground` | `Ground = SNOW_RUGGED DeepSnow` | the surface of a `Terrain.ini` texture `Class` |
| `DefaultSurface` | `Open` | tiles no `Tile` or `Ground` line covers |
| `Road` | `Road = DirtRoad Track` | the surface of the map's roads whose `Roads.ini` name starts so; other roads (decals: arrows, cracks, rails) change nothing |
| `BridgeSurface` | `Paved` | on a bridge |

## How it plugs in

- **Hub:** `unitSpeed` (top speed, the slope and the ground) and a new general point, `unitHandling(object, Handling, value)`, from `Locomotor::getMaxAcceleration`, `getBraking` and `getMaxTurnRate`; `objectDestroyed`, `gameStartAfterMap`.
- **Surfaces** (`Engine/Surfaces.cpp`): at every game start (`gameStartAfterMap`, a loaded save too) a grid of 10-unit cells (`MAP_XY_FACTOR`) gets each cell's surface from its tile, and then the roads are painted over it as strips of their drawn width. A unit's speed reads its cell. The tiles and roads come from the map file and the CRC-checked `Terrain.ini` and `Roads.ini`, so every machine and replay reads the same grid. `-cohLog` lists each surface's cells per map.
- Only vehicles whose current locomotor drives on the ground (treads, four wheels, a motorcycle, `OTHER`) are changed; `HOVER`, `WINGS`, `THRUST` and legs are left alone.
- Nothing is saved: the weights are worked out from the templates and the upgrade masks, which are.

## Public header

`VehicleMovement.h`: `isActive()`, `weightOf(vehicle)` (tons with its upgrades, or nothing for what has no weight or while the flag is off).

## Engine notes

- `Locomotor` (`Z/GameLogic/Object/Locomotor.cpp`): every move mode reads its limits through `getMaxSpeedForCondition`, `getMaxAcceleration`, `getBraking` and `getMaxTurnRate` (870-950); the move modes are `moveTowardsPositionTreads` 1267, `Wheels` 1381, `Legs` 1717, `Climb` 1813, `Wings` 1944, `Hover` 1986, `Thrust` 2014, `Other` 2449. Treads turn in place (`rotateTowardsPosition`) and slow down by how far they have to turn; wheels steer through a turn radius from the turn rate (`calcMinTurnRadius` 1690). Braking also sets how early a vehicle starts slowing for its goal (`maxSpeed / getBraking()`), so heavier vehicles start braking earlier and still stop on the spot.
- The locomotor's `m_maxSpeed`, `m_maxAccel`, `m_maxBraking`, `m_maxTurnRate` (`setMaxSpeed` and the rest) are caps only: they never raise the template's values, and they are saved with the locomotor. That is why weight goes through the hub and not `AIUpdateInterface::chooseLocomotorSetExplicit`, where the old Economy slice capped speed.
- Retail has no slope slowdown for ground vehicles: only `moveTowardsPositionClimb` slows on a slope.
- Terrain tiles: `TheTerrainVisual->getTerrainTile(x, y)` returns the `TerrainType` (`Core/GameEngine/Include/Common/TerrainTypes.h`: `getName`, `getClass`) of the cell's base tile (`WorldHeightMap::getTerrainNameAt`, blends ignored). It is loaded in `-headless` games too (`W3DTerrainVisual::load` makes the full height map either way), and `BuildAssistant` already asks it from logic. It is a string search per call, so the slice reads it once per cell at game start. Class names for INI need `DEFINE_TERRAIN_TYPE_NAMES` before including `TerrainTypes.h`.
- Roads: drawn by the client's `W3DRoadBuffer`, which `-headless` games never make; logic keeps no road list. The map's objects (`MapObject::getFirstMapObject()`, kept until `W3DTerrainLogic::reset`) hold each road segment as a `FLAG_ROAD_POINT1` object followed by a `FLAG_ROAD_POINT2` one, named after the `Roads.ini` road (`TheTerrainRoads->findRoad`); its drawn half width is `getRoadWidth() * getRoadWidthInTexture() / 2` (`W3DRoadBuffer.cpp` 384). Many `Roads.ini` "roads" are decals (arrows, cracks, crossings, rails), so roads count only by name.
- The pathfinder's cells are only clear, water, cliff, rubble, obstacle and impassable (`PathfindCell.h` 42); nothing about the ground's texture.
- Upgrades: an object's own are `Object::getObjectCompletedUpgradeMask`, its player's `Player::getCompletedUpgradeMask`. Whether a player upgrade changes an object: one of its behavior modules' `getUpgrade()->testUpgradeConditions(mask)`, but not a module that also says yes to an empty mask: `UpgradeMux::testUpgradeConditions` returns true for a module with no triggers (only conflicts), which made Dozers and Tomahawks carry Composite Armor's weight.
- Composite Armor (`Upgrade_AmericaCompositeArmor`, a player upgrade) is answered to by the Crusader, Paladin, Microwave tank and the generals' Avengers; the Overlord's riders are object upgrades (`Upgrade_ChinaOverlordGattlingCannon`, `...PropagandaTower`, `...BattleBunker`); the Emperor answers only to the gattling and has its tower built in.

## Testing

- Unit tests: `z_googletest --gtest_filter=VehicleMovement*` (the curves, the slope rule, upgrade weight, name matching, the surface grid and road strips).
- Surfaces per map (`-cohLog`): Tournament Desert 204 by 270 cells, 62 road segments: 774 paved, 6065 track, 4887 rock, the rest open; Twilight Flame 9 roads, 26k rock cells; Winter Wolf 106 roads, almost all snow.
- Test folder `C:\Users\ibm21\Games\ZH-VehicleMovement` (retail hardlinked, own exe): `install.ps1 -GameDir`, `skirmish.ps1 -GameDir`, `replaycheck.ps1 -GameDir`. `-cohLog` writes each vehicle's weight and factors whenever they change.
- Flags-on replay: `Replays\CoHVehicleMovement\vehiclemovement-surfaces-allflags-usa-china-desert-11min.rep` (Sectors, GeneralsArsenal, Upkeep, Squads, Accuracy, FireRate, Pacing, DirectionalArmor, UnitScale, Veterancy, VehicleMovement; Hard AIs, America against China, game over at 11 minutes) plays clean, as do `Replays\CoH` (flags off), `CoHVeterancy`, `CoHDirectionalArmor` and `CoHUnitScale`. An earlier recording (weights only) showed Paladins going from 45 to 49 t when Composite Armor was done; it is kept in `stale-2026-10-19`.
- A second test folder, `ZH-VehicleMovement2`, for builds while a watch game runs in the first.
