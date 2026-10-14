# Demo

A ready-made test setup at game start, for whichever features are on. Feature flag: `Demo` (it was `SquadsDemo`, and kept its place in the flag mask, so old demo replays still play back).

## Starting it

`.\IbmEngine\scripts\demo.ps1` (or `generalszh.exe -quickstart -demo`) skips the menus and starts a skirmish straight away: you (your skirmish preferences' name, side and colour) against one easy AI on a random side, on the blank **IbmDemo** map, with the Demo flag on. `-demo "<map path>"` uses another map, and `-demoSide FactionGLA` (any PlayerTemplate name) picks your side.

**IbmDemo** (`Maps/IbmDemo/IbmDemo.map`, installed to `Documents\Command and Conquer Generals Zero Hour Data\Maps`, the only place the game looks for new maps; it is also in the skirmish map list) is flat grass, 3000 × 3000, with no props and two start positions 1800 apart. `tools/make_demo_map.py <MapsZH.big> <output>` rebuilds it from the retail Tournament Desert map (whose skirmish sides, teams and lighting it keeps).

## Testing a strike (debug builds)

`generalszh_dbg.exe -win -quickstart -demo -demoSide FactionGLA -demoStrike SuperweaponScudStorm` fires that special power from your Command Center at the enemy's Command Center three seconds in, then `DebugLogFileD.txt` (lines with `IbmEngine demoStrike`) lists every object it creates and follows every projectile, missile and launcher, and the game quits after 40 seconds. `-demoStrikeAt <Object>` aims at the first enemy object of that kind instead, and the log also follows those objects (airborne, stealth, AI state and victim). The log starts with the buttons of the building that fires. Run it windowed: headless games don't run powers.

## What it places

Every human player of a skirmish or LAN game gets $100,000, and these groups toward the map's middle from their Command Center:

| Feature on | Your group | The first enemy player's group, further on |
|---|---|---|
| `Squads` | All three Barracks, one of each squad, a Humvee, Troop Crawler, Technical and Chinook | A Rebel, Red Guard and Ranger squad, a Battlemaster, a Technical, a Comanche and a Supply Center |
| `Economy` (beside the Squads group) | Every faction's War Factory, Airfield and tech buildings (Strategy Center, Propaganda Center, Palace), four power plants, the Paladin, Nuke Cannon, Scud Launcher and Marauder sciences, one of each tank and helicopter, plus a Dozer and a Supply Truck (retail stats, to compare) | Two Crusaders, two Battlemasters, an Overlord, two Scorpions, a Marauder and a Helix |
| `WatchZones` (beside the Squads group, on the other side from Economy) | A USA and a China Airfield, a GLA Tunnel Network and two power plants, so every side's watch powers are on hand | Two Comanches, a Helix, two Battlemasters, a Crusader and a Scorpion to watch for |

- `AirPower`: you start at general rank 5 with its promotion points to spend, and your side's new power (Precision Strike, Napalm Strike or Mine Rockets) and rank-5 super weapon strike are already on the Command Center; the points buy the retail powers.
- `Accuracy` and `Armor` need nothing of their own: shoot at the enemy groups.
- Every tank can be built, so its cost and build time can be checked on the buttons.
- With none of Squads, Economy, AirPower and WatchZones on, Demo does nothing.
- It is placed inside the game logic, so replays work; loading a saved game doesn't place it again.

## Files

| File | What it does |
|---|---|
| `Demo.cpp`, `Demo.h` | The setup |
| `DemoLaunch.cpp` | `-demo`: a skirmish straight from startup; `-demoSide` |
| `DemoTest.cpp` | `-demoStrike`: the strike test (debug builds) |
| `Maps/IbmDemo/IbmDemo.map` | The blank demo map |
| `tools/make_demo_map.py` | Builds the demo map |

Hooks: `GameLogic::startNewGame` calls `IbmEngine::setUpDemo()` after the starting bases are placed; `GameEngine::init` calls `IbmEngine::startDemoGame()` once the map list is built; `GameLogic::update` calls `IbmEngine::updateDemoTest()` once a frame.
