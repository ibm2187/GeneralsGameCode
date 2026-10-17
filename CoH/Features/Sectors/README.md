# Sectors

**Flag** `Sectors` · **Task** [01](../../Docs/Tasks/01-Sectors.md) · **Public header** [Sectors.h](Sectors.h)

Supplies come from territory. The map is divided into sectors around capture points, units take a point by standing on it, and every held sector joined to its owner's home pays money. There are no supply centers, supply trucks, supply stashes, drop zones or gathering. Money is the only resource.

## How it plays

- **Sectors from any map.** At the start of a skirmish or multiplayer game, every supply source (docks, warehouses, piles) and every tech building becomes a source. Sources close together form one point, and a point with an oil derrick or another tech building is high-value. Each player's start position is a home point, and sources close to it join the home. Each point owns the ground nearest to it, so the whole map is divided. Ground close to a start always belongs to its home, so a base has room. Sectors that touch are neighbors.
- **Home sector.** Each player owns the home sector from the first frame. It can't be captured while the player has a command center standing.
- **Capture by conquest.** Combat units inside a point's radius move its capture toward their side. Which kinds count is set in INI: infantry and vehicles, but not aircraft, dozers, workers or drones. Enemy units on the same point contest it, and nothing moves. An enemy point is neutralized first, then captured. More units capture faster, up to a cap. A point nobody stands on drifts back to where it was. There is no capture upgrade.
- **Tech buildings** belong to whoever holds their sector. A building taken with a capture ability goes back to the sector's owner. The oil derrick's own income is gone; its sector pays instead.
- **Income.** Every few seconds each held sector pays its owner, by type (supply, high-value, home), but only if a chain of the owner's own sectors joins it to a root: the home sector or a sector with one of the owner's command centers. A held sector that isn't joined is **cut off** and pays nothing.
- **Building** is allowed only in owned sectors. Base defenses may also go in a neutral sector next to an owned one. Both rules are set in INI. The placement cursor shows the spot as illegal, and the AI's base building follows the same rule.
- **Removed:** supply centers, supply stashes and supply drop zones are gone from every Dozer and Worker (all generals). Prerequisites on them now ask for the same general's Barracks, so the War Factory, Airfield, Arms Dealer and Chinook stay buildable. The supplies on the map are emptied at game start, so nothing can gather: the Worker and the Dozer only build. Where the Chinook is built is task 02's.
- **Skirmish AI.** Every few seconds each AI player sends idle combat units to its best targets: its sectors with enemies on them, then the neutral and enemy sectors bordering its connected territory (first those that join its cut-off sectors back up), high-value and near ones first. It retakes lost sectors the same way.
  - Its scripts wait for supply buildings ("War Factory once a Supply Center is built"). At game start their conditions on supply buildings are made to ask about the same general's Barracks (`AIScriptRename`), as the pack's prerequisites do.
  - On every AI step, supply buildings come out of its build list (its scripts keep adding them), and an unbuilt building planned where the build rule says no is pulled back toward its home along the same line. A priority entry it can't place would otherwise hold up everything else it builds.
  - So it builds no gatherers.
- Skirmish and network games and their replays have sectors. The campaign has none; the pack still applies there.

## UI

- On the ground, in the style of Company of Heroes' territory lines:
  - Each player's territory (all their sectors together) has one smooth, connected outline that follows the ground: a crisp line in the owner's color just inside it, and a soft band that fades from that color to nothing further in. The band narrows where the outline turns tightly, so it never folds.
  - The neutral ground's outline is a dim dashed grey line. Between two sectors of the same owner, or two neutral ones, there is only a faint line (dashed when neutral).
  - When a sector changes hands, its edge flashes white for a few seconds.
  - Hills hide the lines and rings behind them.
  - Each point has a ring in its owner's color; the capture progress is an inner arc in the capturing side's color. A contested ring blinks white; a cut-off sector's ring is broken. A high-value point has a second, inner ring.
- On the minimap: a light tint of the owner's color over each owned sector, the smooth borders in the colors of the sectors on each side (faint between two sectors of one owner), and a dot per point, hollow when cut off.
- The money tooltip: income per minute from sectors, with how many are held and cut off.
- Messages "Sector captured" and "Sector lost" for the local player (labels `CoH:SectorCaptured`, `CoH:SectorLost`, `CoH:SectorIncome`; English text is built in).

## INI

The pack is `Data/INI`:

| File | What |
|---|---|
| `10_Sectors.ini` | the `CoHSectors` block (all tuning) and the `CoHSectorPoint` object |
| `20_CommandSets.ini` | generated: empties the supply building slots of every Dozer and Worker command set (`N = None`) |
| `30_Prerequisites.ini` | generated: prerequisites on supply buildings become the general's Barracks |
| `40_TechBuildings.ini` | generated: removes the oil derrick's own `AutoDepositUpdate` |

The generated files come from the retail `INIZH.big`: `python CoH/Features/Sectors/Data/make_pack.py ["<Zero Hour folder>"]`.

`CoHSectors` settings (defaults in `Engine/Config.cpp`, pack values in `10_Sectors.ini`):

| Setting | Pack | Meaning |
|---|---|---|
| `MergeDistance` | 250 | sources this close, in a chain, form one point |
| `HomeMergeDistance` | 500 | a point this close to a start joins the home |
| `CellSize` | 40 | grid the map is divided on (world units) |
| `HomeRadius` | 450 | ground this close to a start always belongs to its home |
| `CaptureRadius` | 160 | units this close to a point capture it |
| `CaptureInterval` | 0.5 | seconds between capture steps |
| `CaptureSeconds` / `NeutralizeSeconds` | 20 / 15 | one unit takes a neutral point / neutralizes an enemy one in this time |
| `RecoverSeconds` | 60 | an empty point drifts back in this time; 0: never |
| `MaxCapturingUnits` / `ExtraUnitBonus` | 5 / 0.25 | speed is 1 + bonus per extra unit, up to the cap |
| `CaptureKinds` / `NonCaptureKinds` | INFANTRY VEHICLE / AIRCRAFT DOZER HARVESTER DRONE | which units capture (take AIRCRAFT out for helicopters) |
| `HighValueKinds` | TECH_BUILDING | map buildings that make a point high-value and follow its owner |
| `IncomeSeconds` | 10 | pay period |
| `SupplyIncome` / `HighValueIncome` / `HomeIncome` | 50 / 120 / 75 | pay per period by sector type |
| `BuildInOwnedSectorsOnly` | Yes | build only in owned sectors |
| `DefensesOnNeutralNeighbors` / `DefenseKinds` | Yes / FS_BASE_DEFENSE | defenses may go in a neutral sector next to an owned one |
| `AIThinkInterval` / `AIGroupSize` / `AIMaxTargets` | 3 / 3 / 4 | how often the AI decides, units per target, targets at once |
| `AIDefendWeight`, `AIReconnectWeight`, `AIFrontierWeight`, `AIHighValueWeight`, `AIEnemyOwnedWeight`, `AIDistanceWeight` | 1000, 400, 300, 150, -50, -200 | how the AI ranks targets (distance per 1000 units from its home) |
| `AIRemoveFromBuildList` | FS_SUPPLY_CENTER FS_SUPPLY_DROPZONE | taken out of AI build lists |
| `AIScriptRename = <from> <to>` | SupplyCenter Barracks, SupplyStash Barracks, SupplyDropZone Barracks | AI script conditions on those kinds ask about the template named with the first match replaced; one line each |
| `BorderSmoothing` | 3 | rounding passes for the drawn borders; 0 draws the grid's staircase |
| `BorderBandWidth` | 40 | the soft band inside a territory's outline, world units |
| `PointObject` | CoHSectorPoint | the point object |

## Code

- `Domain/`: the rules, unit tested in `Tests/`. `Points` (sources into points), `SectorMap` (dividing the ground, neighbors, borders), `Capture` (occupation, contesting, neutralizing, speed), `Network` (the cut-off walk and income), `BuildRule`, `AiTargets`, `BorderLines` (drawing geometry: the grid's border edges chained into lines and region outlines, rounded through the middle of each straight run and then by corner cutting, resampled, inset into a sector or region, and how deep a band can reach without folding).
- `Engine/`: `Sectors.cpp` (declaration and hook subscribers), `Setup.cpp` (a new game's points, emptying supplies, AI build lists), `SectorPointUpdate` (the point object's module; its state is saved and part of the CRC), `World` (the game's sectors, made from the point objects on the first frame, so loaded games work), `Logic.cpp` (capture, owner changes, income), `AI.cpp` (sending units), `AIScripts.cpp` (script conditions), `AIBuildList.cpp`, `BuildRule.cpp`, `Config` (the INI block), `Public.cpp` (`Sectors.h`).
- `UI/`: `BorderGeometry` (the lines lifted onto the ground and cut into culled chunks: the sector borders once per World build, the territory outlines again only when an owner changes; client only, so the game logic and replays are untouched), `GroundOverlay` (styles, culling, hills hiding lines, the flashes, the rings), `RadarOverlay`, `MoneyTooltip`, `Colors`.

## Hooks

Sectors subscribes to `gameStartAfterMap`, `logicFrame`, `objectCreated`, `gameEnd`, and to these points it added to the hub (listed in the [hooks table](../../README.md#hooks-in-original-code)):

| Hub point | Original code | Used for |
|---|---|---|
| `buildLocation()` (a `Veto`) | `BuildAssistant::isLocationLegalToBuild` | building only in owned sectors |
| `inGameUIPostDraw()` | `InGameUI::postDraw` | borders and rings on the ground |
| `radarDraw(x, y, w, h)` | `W3DRadar::draw` | borders and points on the minimap |
| `drawGradientQuad(...)` (provided, not an event) | `W3DDisplay::init`, `W3DDisplay::drawGradientQuad` | the territory band's gradient |
| `moneyTooltip(player, text)` | `ControlBar::populateBuildTooltipLayout` | income per minute |

It also relies on the CommandSet override fix in `ControlBar` (`// CoH fix`), so its command set changes add up with other packs'.

## Testing

- Unit tests: `z_googletest.exe --gtest_filter=Sectors*`.
- Headless AI-vs-AI game, from the game folder:
  `generalszh.exe -useCwd -headless -coh Sectors -cohSkirmish "Tournament Desert" -cohSide Observer -cohOpponents 2 -cohAI hard -cohLog -cohQuitAfter 600`.
  `CoH.log` in the user data folder lists the points, every change of hands, and once a minute each sector's state, each player's income and each AI's money, buildings, units and targets.
- The game's replay (`Replays\00000000.rep`) plays back with Sectors on (the flag is in its header): `CoH\Scripts\replaycheck.ps1 -Replays 00000000.rep` checks that sector play is deterministic.

## Engine notes

- `InGameUI::postDraw` runs inside `TheDisplay->beginBatch()` (`W3DInGameUI::draw`), so thousands of `drawLine` calls cost one render as long as the texture and mode don't change. The radar hook runs in the same batch.
- `Display::drawLine` is always a screen-space rectangle (`Render2DClass::Add_Line`); its two-color form shades along the line, not across it. Many parallel lines read as stripes, and rectangles fanned round a curve overlap and poke out. `Render2DClass::Add_Quad_HGradient` shades any four corners, which is what the hub's `drawGradientQuad` exposes.
- Drawing over the 3D view has no depth test. Hills hide lines because `GroundOverlay` walks each point's sight line to the camera over `getGroundHeight`. The camera is found where the pick rays of two screen corners meet (`View::screenToWorldAtZ` at two heights), since `View` has no camera position getter. Units and buildings don't hide lines.
- Cost, measured with `-cohLog` (logged every 30 s): about 0.13 ms a frame for the ground drawing on Mountain Guns (16 sectors, hilly) and 0.05 ms on Dark Night (12 sectors, the biggest skirmish map); the sector borders are made in about 1 ms and the territory outlines in under 1 ms when a sector changes hands.
- Points over water: `TerrainLogic::isUnderwater` gives the water height; lines are lifted to it, or the overlay draws them at the sea bed.
