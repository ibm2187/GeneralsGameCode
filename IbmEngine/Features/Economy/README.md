# Economy

Fewer, tougher units: every vehicle and aircraft costs more, takes longer to build, has more health, hits harder but fires slower, and moves slower, so each one is worth more and losing one matters. Feature flag: `Economy`.

| Stat | Vehicles | Aircraft |
|---|---|---|
| Cost | ×2 | ×2 |
| Build time | ×2 | ×2 |
| Health | ×1.5 | ×1.5 |
| Damage | ×1.25 | ×1.25 |
| Rate of fire | ×0.6 | ×0.6 |
| Damage against infantry | ×1.5 | ×1.5 |
| Speed | ×0.8 | ×0.85 |

- Health goes up more than damage, so fights last longer and a hurt unit has time to pull back.
- Shots hit harder but come much slower: ×1.25 damage at ×0.6 rate of fire is 75% of retail damage per second against vehicles, and with health ×1.5 a vehicle lasts twice as long. With the [Armor](../Armor/README.md) feature, deflections stretch that further.
- Vehicle and aircraft weapons do ×1.5 against infantry (`VsInfantry`), so against infantry they do 1.125 times retail damage per second: tanks are a real threat to squads.
- Each tank has its own speed: light units keep more of it (Humvee, Technical, Rocket Buggy ×0.9), main battle tanks get the baseline (Crusader, Battlemaster, Marauder ×0.8), and heavy ones lose the most (Paladin, Tomahawk, Inferno, Scud Launcher ×0.75; Overlord, Emperor ×0.7).
- Scouts, transports and support vehicles (Humvee, Ambulance, Technical, Radar Van, Battle Bus, Combat Bikes, Troop Crawler, Listening Outpost) cost and build only ×1.5.
- Dozers, supply trucks, Chinooks and drones keep their retail stats (`Exempt`). So do units no player can build (civilian cars, campaign units).
- Supply income and buildings are unchanged, so units become scarcer compared with the base.
- Damage and rate of fire multiply the unit's total weapon bonus, so they stack on top of veterancy and upgrades. Rate of fire stretches both the delay between shots and the clip reload.
- Speed caps the unit's normal top speed. Its speed when badly damaged stays the retail value, unless that is faster.

## The IbmEconomy block

```ini
IbmEconomy
  Kind = AIRCRAFT Cost:200% BuildTime:200% Health:150% Damage:125% RateOfFire:60% VsInfantry:150% Speed:85%
  Kind = VEHICLE  Cost:200% BuildTime:200% Health:150% Damage:125% RateOfFire:60% VsInfantry:150% Speed:80%
  Exempt = DOZER HARVESTER DRONE
  Unit = ChinaTankOverlord Speed:70%
End
```

- `Kind` applies to units a player can build (BuildCost above 0) with that KindOf. The first matching line wins, so `AIRCRAFT` comes before `VEHICLE` (helicopters have both).
- `Exempt` units get no `Kind` multipliers.
- `Unit` replaces only the stats it names, for that object and for the generals' versions of it: `Tank_ChinaTankOverlord` uses the `ChinaTankOverlord` line unless it has its own.

## Files

| File | What it does |
|---|---|
| `Economy.cpp`, `Economy.h` | The multipliers and the `IbmEconomy` INI block |
| `Data/INI/Features/Economy/10_Economy.ini` | Every number above |

Hooks: `ThingTemplate::calcCostToBuild` and `calcTimeToBuild` (cost and build time, which the build buttons, AI, refunds and selling all use), `ActiveBody`'s constructor (health), `Weapon::computeBonus` (damage and rate of fire), `WeaponTemplate::dealDamageInternal` (damage against infantry), `AIUpdateInterface::chooseLocomotorSetExplicit` (speed); INI.cpp registers the block.

## Testing

- With the flag off every multiplier is 1, so games and replays play as before.
- The Crusader costs $1800 and takes 20 s to build; an Overlord has 1650 health; a Humvee costs $1050.
