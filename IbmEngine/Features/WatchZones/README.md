# WatchZones

Airfields and Tunnel Networks get powers that watch a circle on the map for a while. Jets come in at enemies that enter it, or hidden soldiers wait there and shoot. Feature flag: `WatchZones`.

The powers belong to buildings, not promotions. A power is ready once its building stands and recharges on one timer for all of the player's buildings of that kind. Losing every such building loses the power. Select the building and click the button, then pick the spot (any ground you have explored).

## Powers

| Side | Building | Power | What it does | Reload |
|---|---|---|---|---|
| USA | Airfield (every general's) | Raptor Patrol | 45 s over a circle of 200: each enemy aircraft that enters gets a Raptor pass, 2 air-to-air missiles of 100. Up to 3 passes, 5 s apart (600 at most) | 3 min |
| USA | Airfield | A-10 Watch | 45 s over a circle of 175: enemy ground vehicles that enter get the retail A-10 strike's pass with 4 missiles of 200 (retail fires 6) and a cannon run. Up to 2 passes, 8 s apart (1,600 at most) | 4 min |
| China | Airfield (every general's) | MiG Patrol | 45 s over a circle of 200: enemy aircraft get a pair of MiGs, each after its own target, each firing 2 missiles of 75. Up to 3 passes, 5 s apart (900 at most) | 3 min |
| China | Airfield | MiG Hunters | 45 s over a circle of 175: enemy ground vehicles get a pair of MiGs, each firing 2 armour-piercing missiles of 200 that leave Black Napalm burning. Up to 3 passes, 6 s apart (2,400 at most, plus fire) | 4 min |
| GLA | Tunnel Network (every general's) | Stinger Nest | A camouflaged Stinger Site appears at the spot for 60 s. Its 3 soldiers fire at aircraft only and come back 10 s after dying while the site stands; when it goes, they go | 3 min |
| GLA | Tunnel Network | Trap Field | 10 hidden demo traps at random spots within 140 for 60 s. Each blows up when an enemy vehicle comes within 40: 300, and 150 within 50. Infantry and aircraft don't set them off; detectors find them | 3 min |

- China is meant to be a bit stronger than the USA: pairs and napalm against single jets.
- **Jets** appear about 800 from their target (300, plus the pass's own run-up), on the side of the map edge nearest the building that ordered the power, and fly in. A jet follows the enemy it was sent at until it fires: its target point and its circle on the ground move with the enemy, and its missiles home in. It gives up following (and strikes where it last saw the enemy) if the enemy dies, hides, or gets more than 150 beyond the circle. Each jet goes after the nearest enemy that no other jet of the zone is after; a pair flies side by side, 40 apart. A jet is a real aircraft that can be shot down.
- **The circle** is shown to its owner only. The zone sees what is inside it (vision 200 or 175), but doesn't detect stealth.
- **Stinger Nest and Trap Field** objects can't be selected or ordered. The nest and its soldiers are stealthed from the start (retail needs Camo Netting); firing reveals the soldiers. Traps hit enemies only, so they don't set each other off.
- On the **Tunnel Network** the two powers take the last two exit buttons (slots 9 and 10): every slot was in use. The tunnel still holds ten; Evacuate lets out the units that have no exit button.
- The powers aren't on the power bar (the USA one is full); select the building.

## Module: IbmWatchZoneUpdate

The zone is an invisible, untouchable object with this module; see `WatchZoneUpdate.h` for its fields (`Radius`, `Duration`, `TargetKindOf`, `ForbiddenKindOf`, `Targets = ANY | AIRBORNE | GROUND`, `Strikes`, `JetsPerStrike`, `JetSpacing`, `StrikeDelay`, `StrikeOCL`, `ApproachDistance`, `FollowRange`, `ZoneDecal`). `StrikeOCL` is a `DeliverPayload` with one transport. The building's `OCLSpecialPower` creates the zone with `CreateLocation = CREATE_AT_LOCATION`, which records the building as the zone's producer (the jets' home edge).

Which enemy each jet follows is kept in memory only: after loading a saved game, jets already in the air fly to where their enemy was.

## Files

| File | What it does |
|---|---|
| `WatchZones.h` | The functions the hooks call |
| `WatchZoneUpdate.cpp`, `WatchZoneUpdate.h` | The zone module, and which enemy each jet follows |
| `Data/INI/Features/WatchZones/10_Units.ini` | Weapons, the patrol jets (DeliverPayload copies of the Raptor and MiG), the Stinger Nest and the traps |
| `Data/INI/Features/WatchZones/20_Zones.ini` | The jets' passes, the zone objects and what each power creates |
| `Data/INI/Features/WatchZones/30_Powers.ini` | Special powers, buttons and the buildings' power modules |
| `Data/INI/Features/WatchZones/40_CommandSets.ini` | The buttons on the buildings |
| `Data/INI/Features/WatchZones/WatchZones.str` | Names and tooltips |

Hooks:
- `SpecialPowerType.h`, `SpecialPower.cpp`: two new power types, `SPECIAL_IBM_WATCH_AIR` and `SPECIAL_IBM_WATCH_GROUND` (appended).
- `ActionManager::canDoSpecialPowerAtLocation`: they may target any explored ground. AirPower's `StrikesNeedSight` doesn't apply.
- `DeliverPayloadAIUpdate.cpp`: while approaching, a zone's jet follows its enemy; it fires its weapon, and launches its visible missiles, at the enemy itself.
- `ModuleFactory.cpp`: registers `IbmWatchZoneUpdate`.
- `ObjectCreationList.cpp`: `CreateObject` takes `ScatterRadius`: each object at a random spot within it (synced random). The Trap Field uses it.
- `ControlBar.cpp`: fix: a second pack's change to a command set builds on the first's (the Airfield sets are changed by AirPower too).

## Testing

`generalszh_dbg.exe -win -ignoreAsserts -quickstart -demo -demoStrike IbmWatchA10Watch -demoStrikeAt ChinaTankBattleMaster` fires the power from your building at the first enemy of that kind three seconds in. `DebugLogFileD.txt` then lists the building's buttons, the zone (`IbmEngine watchZone`), every jet and GLA object, and the aimed-at units each frame. Add `-demoSide FactionChina` or `FactionGLA` for the other sides' powers. See the Demo README.

## Known limits

- The skirmish AI doesn't use these powers (its scripts only fire powers they name).
- No cost per use yet; the powers cost only their reload. To revisit with the economy.
- Balance is a first guess: damage, passes, radius, reloads.
