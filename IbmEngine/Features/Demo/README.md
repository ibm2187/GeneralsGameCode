# Demo

A ready-made test setup at game start, for whichever features are on. Feature flag: `Demo` (it was `SquadsDemo`, and kept its place in the flag mask, so old demo replays still play back).

Every human player of a skirmish or LAN game gets $100,000, and these groups toward the map's middle from their Command Center:

| Feature on | Your group | The first enemy player's group, further on |
|---|---|---|
| `Squads` | All three Barracks, one of each squad, a Humvee, Troop Crawler, Technical and Chinook | A Rebel, Red Guard and Ranger squad, a Battlemaster, a Technical, a Comanche and a Supply Center |
| `Economy` (beside the Squads group) | Every faction's War Factory, Airfield and tech buildings (Strategy Center, Propaganda Center, Palace), four power plants, the Paladin, Nuke Cannon, Scud Launcher and Marauder sciences, one of each tank and helicopter, plus a Dozer and a Supply Truck (retail stats, to compare) | Two Crusaders, two Battlemasters, an Overlord, two Scorpions, a Marauder and a Helix |

- `Accuracy` needs nothing of its own: shoot at the enemy groups.
- Every tank can be built, so its cost and build time can be checked on the buttons.
- With neither Squads nor Economy on, Demo places nothing.
- It is placed inside the game logic, so replays work; loading a saved game doesn't place it again.

## Files

| File | What it does |
|---|---|
| `Demo.cpp`, `Demo.h` | The setup |

Hook: `GameLogic::startNewGame` calls `IbmEngine::setUpDemo()` after the starting bases are placed.
