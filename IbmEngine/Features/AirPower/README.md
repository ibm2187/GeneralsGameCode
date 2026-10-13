# AirPower

Players build no jets and no super weapons. Airfields build helicopters only, air strikes come from general powers, and the super weapons become a rank-5 strike fired from the Command Center. Strikes need a spotter. Feature flag: `AirPower`.

## Airfields

- **USA airfields** (and the Air Force, Superweapon and Laser generals' versions) lose the Raptor, Aurora and Stealth Fighter, and the Bunker Busters upgrade (a Stealth Fighter upgrade). Comanches and their upgrades stay.
- **China airfields** (and the Nuke, Infantry and Tank generals' versions) lose the MiG. Helixes, aircraft armor and mines stay.
- **GLA** has no airfield and gets no helicopters.
- **The skirmish AI**: a team that asks for a jet builds a helicopter instead (Raptor, Aurora and Stealth Fighter → Comanche, MiG → Helix), in the same general's version when there is one.
- The airfield model stays for now; it is meant to be remodelled as a helipad later.

## New general powers

| Side | Promotion | Power | What it does |
|---|---|---|---|
| USA | rank 1 (replaces the Stealth Fighter) | Precision Strike | A B-52 drops one heavy bomb: 1000 damage within 25, 250 within 60. 4-minute reload |
| China | rank 3 | Napalm Strike | Five MiGs napalm the spot. Retail had this power on the Command Center but never sold it. 4-minute reload |
| GLA | rank 3 | Mine Rockets | Four rockets come down around the spot, each laying a ring of cluster mines (radius 45). 4-minute reload |

They are on the base sides' promotion screens, Command Centers and power bars; the generals' versions don't offer them yet.

## Super weapons

The Particle Cannon, Nuclear Missile silo and Scud Storm can't be built (their builder buttons are gone, which also stops the skirmish AI building them). At **rank 5** every player gets its side's strike, ordered from the Command Center (every general's version) with its own button and power-bar slot. The missiles come down out of the sky onto the target, as if launched from off the map: they appear 1000 above it and land about 5 seconds later, so the enemy gets a moment to move.

| Side | Strike |
|---|---|
| USA | Cruise Missile (the Superweapon General's MOAB-sized blast; the Particle Cannon's beam only works from its building). 6-minute reload |
| China | Nuclear Missile (the silo's nuclear blast) |
| GLA | Scud Storm: nine Scuds within 120 of the spot, in the Scud Storm's own pattern |

The USA power bar's Particle Cannon slot fires the Cruise Missile, and its Spy Satellite slot (the scan is still on the Command Center) holds Precision Strike.

## Strikes need a spotter

With `StrikesNeedSight = Yes`, a human player's damaging powers (strikes, bombs, missiles, mines, artillery) can only target ground that one of their units or buildings sees right now. Retail only refused never-explored ground. The skirmish AI is exempt, since its scripts aim on their own.

## Files

| File | What it does |
|---|---|
| `AirPower.cpp`, `AirPower.h` | The AI's jet-to-helicopter swap, the sight rule and the `IbmAirPower` INI block |
| `Data/INI/Features/AirPower/10_CommandSet.ini` | Airfields without jets and builders without super weapons (`None` empties a slot) |
| `Data/INI/Features/AirPower/20_AirPower.ini` | `StrikesNeedSight`, and `AIReplace = <jet> <helicopter>` lines |
| `Data/INI/Features/AirPower/30_Powers.ini` | The new powers: weapons, falling warheads, deliveries, sciences, powers, buttons, Command Center modules, and the rank-5 grant |
| `Data/INI/Features/AirPower/40_PowerCommandSets.ini` | Where the power buttons go: Command Centers, power bars, promotion screens |
| `Data/INI/Features/AirPower/AirPower.str` | Names and tooltips |

Hooks: `TeamTemplateInfo` (Team.cpp) swaps the AI's jets; `CommandSet::parseCommandButton` (ControlBar.cpp) accepts `None` to empty a slot; `ActionManager::canDoSpecialPowerAtLocation` applies the sight rule; INI.cpp registers the `IbmAirPower` block.

## Known limits

- The new promotions (Precision Strike, Napalm Strike, Mine Rockets) are only on the base USA, China and GLA; the generals' promotion screens are unchanged.
- The skirmish AI doesn't use the new powers or the rank-5 strikes (its scripts name the old ones).
- Mine Rockets lay China's cluster mines (the same look).
