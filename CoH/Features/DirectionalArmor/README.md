# DirectionalArmor

Vehicles have front, side and rear armor and weapons have penetration, like in Company of Heroes, tuned to be close to real armor. Tanks are deadly to infantry in the open. Flag: `DirectionalArmor`. Task: [07](../../Docs/Tasks/07-DirectionalArmor.md).

> A hit on a vehicle penetrates with **chance = penetration ÷ armor** of the side it hits (always, when penetration is at least the armor).
> A shot that doesn't penetrate is **deflected**: it does 5% of its damage, throws sparks off the hull and clangs; anti-tank shots also float "Deflected".
> Then **where it hit** scales the damage: **75% on the front, 125% on the side, 150% on the rear**.

- **Close to real:** a tank's front is about twice as thick as its sides and three to four times its rear. A hit on the side or rear reaches the crew, ammunition, engine and fuel, so it does more than one on the thick front plate.
- **Side hit:** where the shooter is (a rocket's or shell's launcher), seen from the vehicle's hull: a 90° front arc, a 90° rear arc, the sides between. If the shooter is gone, where the projectile is.
- **Penetration by range:** shaped charges (rockets, missiles) penetrate the same at any range; kinetic rounds (tank shells, bullets) fall from their near value at point-blank to their far value at full range.
- **Area damage always gets through:** weapons with a damage radius over 30 (artillery, bombs, super weapons).
- **Never rolls:** weapons whose DamageType has no `Penetration` line and that have no `Weapon` line: flame, poison, radiation, microwaves, crushing, the pilot snipe, particle beams.
- **No armor:** infantry, buildings and aircraft (`Exempt = AIRCRAFT`). Any other vehicle without a `Unit` line gets `Kind = VEHICLE 60 40 25`.
- **Tanks against infantry:** tank guns, vehicle machine guns and the Overlord's gattling obliterate soldiers: much more damage (`InfantryDamage`), a high-explosive blast that falls off from its middle (`InfantryBlast`, `20_TankGuns.ini`), and the dead are thrown.
- **Skirmish AI flanks:** an AI vehicle fighting an armored one from in front, where its hits get through less than 75% of the time, drives round to its side (if a hit from there does 15% more damage on average and the drive is under 250), then attacks again. It rests 12 s before flanking again. Its own orders always win.
- Rolls use the synced random numbers (one per roll), so every PC and replay agrees.

## Rockets against tanks

The user's notes: "Rocket launchers do a lot of damage from the front"; then, after watching the first version, "rear hits and side hits should do more damage; frontal should do less damage" and "the goal is to be close to realistic". A rocket soldier (`INFANTRY_MISSILE`, a shaped charge: penetration 55 at any range) against a Crusader (160 / 85 / 45):

| Side | Gets through | Average damage of a rocket: retail | first version | now |
|---|---|---|---|---|
| Front | 34% | 100% | 43% | 28% |
| Side | 65% | 100% | 57% | 83% |
| Rear | 100% | 100% | 88% | 150% |

So a rocket from behind does more than five times what one from the front does: a tank shows its front to rocket soldiers, and flanking them pays.

## Tanks against infantry

The user's notes: "We also need tanks to do a lot more damage against infantry" and "tanks should shoot like CoH; obliterating infantry." Retail `HumanArmor` (every infantry armor alike) takes 10% from `ARMOR_PIERCING`, the tank guns' damage type, so a tank shell did 6 to a soldier and had no blast. Now:

- `InfantryDamage` multiplies what vehicles' weapons do to infantry, on top of its armor: tank guns ×20 (10% becomes 200%), machine guns ×2.
- `20_TankGuns.ini` gives every tank shell a high-explosive blast (`SecondaryDamage` over `SecondaryDamageRadius`). Against soldiers it is scaled by `InfantryBlast`: ×3 just outside the direct hit, falling to ×1 at the blast's edge, as CoH's falloff. Against vehicles it is only the plain splash (a third of a direct hit).
- `DeathType = EXPLODED`: soldiers a shell kills are thrown by the blast (their own retail death with `FlingForce`). Vehicles die from it as before.

Damage to one soldier, before FireRate (which keeps damage per second the same) and Accuracy:

| Weapon | Retail, direct hit | Now, direct hit | Retail blast | Now, blast: beside it, at the edge (radius) |
|---|---|---|---|---|
| Crusader, Paladin, Battlemaster, Marauder gun (60) | 6 | 120 | none | 120, 40 (20) |
| Overlord gun (80, retail 20 over 10) | 8 | 160 | 2 | 180, 60 (25) |
| Laser Crusader gun (80) | 8 | 160 | none | 150, 50 (20) |
| Tank General's Battlemaster gun (40) | 4 | 80 | none | 90, 30 (20) |
| Scorpion gun (20, upgraded 25) | 2 / 2.5 | 40 / 50 | none | 48 / 60, 16 / 20 (15) |
| Militia tank gun (35) | 3.5 | 70 | none | 72, 24 (15) |
| Humvee gun, Sentry Drone (10, 8; `COMANCHE_VULCAN`) | 10 / 8 | 20 / 16 | | |
| Gattling tank (15), technical MG (10), Overlord gattling (10) (`GATTLING`) | 15 / 10 / 10 | 30 / 20 / 20 | | |
| Quad Cannon (10, upgraded 8), combat bike MG (8) (`SMALL_ARMS`) | 10 / 8 | 20 / 16 | | |

A Rebel or Red Guard has 120 health and a Ranger 180 (no merged flag changes infantry health). So a tank shell's direct hit kills a Rebel, and soldiers right beside it die or are left nearly dead; those at the blast's edge take a third. Retail tank guns also scatter against infantry (`ScatterRadiusVsInfantry = 10`) and, with Accuracy, miss beside soldiers: the blast still catches them.

What keeps infantry alive: heavy cover (Cover 05 scales the damage before this slice), spreading out (the edge of the blast does a third), retreating out of range (Retreat 11), and anti-tank soldiers on the tank's side or rear (above). The blast also hits the shooter's own soldiers (`RadiusDamageAffects = ALLIES ENEMIES NEUTRALS` in retail).

## Penetration and armor

| Weapon kind (DamageType) | Near / far |
|---|---|
| Rifles, light machine guns (SMALL_ARMS) | 8 / 4 |
| Snipers | 10 / 8 |
| Machine guns (GATTLING) | 25 / 12 |
| 20 mm cannons: Humvee, Comanche (COMANCHE_VULCAN) | 35 / 20 |
| Vehicle rockets and missiles (EXPLOSION, shaped charges) | 70 at any range |
| Rocket soldiers (INFANTRY_MISSILE, shaped charges) | 55 at any range; laser-guided Missile Defender 80 |
| Tank guns (ARMOR_PIERCING, kinetic) | 160 / 120; Scorpion 100 / 75 (upgraded 110 / 80), Militia 100 / 75, Battlemaster and Marauder 150 / 110 (upgrades 160 / 120, 170 / 130), Paladin 175 / 135, Overlord 220 / 170 |
| Lasers | 160 / 140 |
| Jet missiles (shaped charges) | 180 at any range |

| Armor (front / side / rear) | Units |
|---|---|
| 5 to 20 | Combat Bikes, Technical, Rocket Buggy, Radar Van, Bomb Truck, Humvee, Ambulance, Toxin Tractor, Supply Truck, drones |
| 40 to 60 front | Dozers, Tomahawk, Listening Outpost, Inferno Cannon, Scud Launcher, Troop Crawler, Quad Cannon, Battle Bus, Nuke Cannon; any other vehicle 60 / 40 / 25 |
| 100 to 120 front | Gattling, ECM, Scorpion 100 / 55 / 35; Dragon 110 / 60 / 40; Microwave 120 / 65 / 35 |
| 130 to 180 front | Avenger 130 / 70 / 40, Marauder 140 / 75 / 45, Battlemaster 150 / 80 / 45, Crusader 160 / 85 / 45, Paladin 180 / 95 / 50 |
| 240 / 130 / 70 | Overlord, Emperor |

So a Crusader's gun always gets through a Battlemaster's front up close (160 against 150) and 80% at full range; a Battlemaster's gun gets through a Crusader's front 94% up close and 69% at full range, and its side and rear always. A rifle gets through a Crusader's front 5% of the time (and the engine's tank armor takes 75% off the rest).

## The CoHDirectionalArmor block

In `Data/INI/10_DirectionalArmor.ini`. The block starts from the defaults: its list lines are the whole list.

| Line | Pack value | Meaning |
|---|---|---|
| `DeflectedDamage` | 5% | share of a deflected shot's damage that still gets through |
| `FrontDamage`, `SideDamage`, `RearDamage` | 75%, 125%, 150% | how hard a hit on each side lands, penetrating or not |
| `FrontArc`, `RearArc` | 90, 90 | full widths of the front and rear arcs, in degrees; the sides are the rest |
| `AreaRadius` | 30 | weapons with a bigger damage radius (primary or secondary) never roll |
| `DeflectFX` | `FX_CoHArmorDeflect` | played on the hull where a shot bounced off: retail `TankStruckSparks`, `FlatSparkSprayLineBurst` and the `VehicleImpactLight` clang |
| `DeflectTextPenetration` | 40 | "Deflected" floats over the vehicle for weapons with at least this penetration (0: never) |
| `Penetration = <DamageType> <near> <far>` | | penetration of a whole kind of weapon |
| `Weapon = <Weapon> <near> <far>` | | one weapon's, and the generals' versions of it (`Nuke_OverlordTankGun`); a line for the version itself wins |
| `Kind = <KindOf> <front> <side> <rear>` | `VEHICLE 60 40 25` | armor of units with no line of their own; the first match wins |
| `Exempt = <KindOf> ...` | `AIRCRAFT` | units with any of these get no `Kind` armor |
| `Unit = <Object> <front> <side> <rear>` | | one unit's armor, and the generals' versions of it (`ChinaTankEmperor` covers `Tank_ChinaTankEmperor`) |
| `InfantryDamage = <DamageType> <percent>` | AP 2000%, SMALL_ARMS, GATTLING, COMANCHE_VULCAN 200% | damage to infantry from attackers of `InfantryDamageFrom` kinds, on top of the infantry's armor |
| `InfantryBlast = <near> <edge>` | 300% 100% | a vehicle's shell blast against soldiers, from just outside the direct hit to the blast's edge |
| `InfantryDamageFrom`, `InfantryDamageNotFrom` | `VEHICLE PORTABLE_STRUCTURE`, `AIRCRAFT` | which attackers get `InfantryDamage` |
| `Flanking` | Yes | the skirmish AI flanks |
| `FlankBelowChance` | 75% | flank only while a hit from where it is gets through less often than this |
| `FlankMinGain` | 15% | and a hit from the flank spot does at least this share more damage on average (penetration, deflection and side damage counted) |
| `FlankRangeShare`, `FlankRearShare` | 80%, 40% | the spot: this share of the attacker's range from the target, swung this far from beside it towards behind it |
| `FlankMaxTravel` | 250 | never drive further than this to get there |
| `FlankCheckEvery`, `FlankTimeout`, `FlankRest` | 1 s, 8 s, 12 s | how often the AI looks, when a flanker attacks again from wherever it got to, how long before it flanks again |

## Files

| File | What it does |
|---|---|
| `DirectionalArmor.h` | the public header: `isActive`, `hasArmor`, `penetrationChanceFrom` |
| `Domain/Armor` | the side a shot hits (a Taylor-series cosine, so every machine agrees), penetration by range, the odds, deflection |
| `Domain/Flank` | when the AI flanks and to where |
| `Domain/Blast` | a shell's blast against soldiers, falling off from its middle |
| `Domain/Names` | which `Unit` or `Weapon` line covers a name (generals' versions) |
| `Engine/DirectionalArmor.cpp` | the feature, its subscriptions, the public answers |
| `Engine/Config` | the INI block; each object's armor and weapon's penetration, looked up once per game |
| `Engine/Combat.cpp` | the hub's `weaponDamage`: the infantry multiplier, then the roll; the log |
| `Engine/Flanking.cpp` | the AI's flanks, from the logic frame |
| `UI/Deflection.cpp` | the sparks, clang and "Deflected" (only where the local player can see) |
| `Data/INI/10_DirectionalArmor.ini` | every number above, the effect and the text |
| `Data/INI/20_TankGuns.ini` | `Weapon` overrides: the tank guns' blast and `EXPLODED` deaths |
| `Tests/` | the angle-to-side mapping, penetration odds, deflection, blast falloff, names, flanking |

Hooks: subscribes to `weaponDamage` (the roll and the multiplier, after Cover), `logicFrame` (flanking, the log every minute) and `gameEnd`. The `weaponDamage` point is shared with Cover (05): it is the same declaration, so whichever slice merges second keeps one copy of it.

## Damage order

For one shot and one victim: Accuracy's hit roll at the fire (`weaponFiringAtObject`), Cover (`weaponDamage`), this slice (`weaponDamage`, after Cover because it is listed after it in `CoH/CMakeLists.txt`), the engine's armor and damage scalar (`ActiveBody::attemptDamage`), Veterancy (`damageReceived`). Every step after the hit roll multiplies the amount, so their order changes only which rolls happen (a shot Cover reduced to nothing doesn't roll here).

## Engine notes

- `WeaponTemplate::dealDamageInternal` (`Weapon.cpp` 1377) deals each victim's damage; for a projectile it runs when the projectile goes off, with the projectile as `source`, and sets `damageInfo.in.m_sourceID` to the launcher just before `attemptDamage`. The hook sits there, so the launcher is known.
- The hull's facing is `Object::getUnitDirectionVector2D()`; a tank's turret turns on its own, the hull only when it moves.
- `Armor.ini` blocks can't be pack overrides: `ArmorStore::parseArmorDefinition` clears and refills the template in place, so a pack's change would outlive the game. Hence `InfantryDamage` in code. `Weapon` blocks are overrides (`WeaponStore::newOverride`) and are dropped when the game ends.
- Retail tank guns have `ScatterRadiusVsInfantry = 10`, so they already miss soldiers; with Accuracy on they roll too (51% for a Crusader on a Ranger at mid range). The splash is what makes them deadly.
- AI units are found with `objectsOf(player)`; a vehicle's fight is `AIUpdateInterface::isAttacking()` and `getCurrentVictim()`. A flank is `aiMoveToPosition(..., CMD_FROM_AI)`, then `aiAttackObject(target, NO_MAX_SHOTS_LIMIT, CMD_FROM_AI)`. A move order's goal is `getGoalPosition()`; when it no longer is the flank spot, the AI ordered something else.
- Flanks in progress aren't saved. After a load a flanker finishes its drive and the AI picks its fights again.

## Testing

- Unit tests: `z_googletest --gtest_filter=DirectionalArmor*`.
- In game: `CoH\Scripts\skirmish.ps1 -Flags Sectors,GeneralsArsenal,Upkeep,Squads,Accuracy,FireRate,Pacing,DirectionalArmor -Windowed`. Hit a tank with rocket soldiers from the front and from behind; send a tank into infantry.
- `-cohLog` writes the rolls by side and by weapon, the deflections, the infantry hits and the AI's flanks to `CoH.log` every minute and at the end.
