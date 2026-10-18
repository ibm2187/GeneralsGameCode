# Accuracy

**Flag** `Accuracy` · **Task** [04](../../Docs/Tasks/04-Accuracy.md) · **Public header** [Accuracy.h](Accuracy.h)

Direct fire can miss. Weapon type, range, target size, movement and veterancy decide the fight more than unit count does.

## How it plays

- Every shot of a direct-fire weapon at an object rolls to hit:
  > hit chance = the weapon's accuracy at that range × target size × shooter moving × target moving × veterancy (× other slices' modifiers), kept between 5% and 90% against units and 95% against buildings
- **Weapon type and range.** Each weapon class has an accuracy at point-blank, at half its range and at its full range, with straight lines in between (`Class` lines, by `DamageType`). Rifles and light machine guns 85/70/50%; gattlings and heavy machine guns spray, 75/55/35%; tank guns 95/85/70%; rockets, RPGs and small shells 90/75/55%; infantry missiles 90/80/60%; jet missiles 100/90/80%; snipers 150/150/140% (over 100% so they still hit small targets). `Weapon` lines give single weapons their own profile.
- **Target size.** The bigger the target, the easier it is to hit, from its bounding radius (`SizePoint` curve): a soldier (7) ×0.6, a Rebel (10) ×0.7, a Humvee or Scorpion (16) ×0.9, a Crusader (18) ×0.95, a technical (20) ×1.0, an Overlord (29) ×1.14, buildings ×1.8 however small (`BuildingSizeMin`). So a tank gun hits a tank far more often than a soldier, and a rifle at long range hits infantry about one shot in three.
- **Buildings** are the easiest targets but are never certain: even a rifle at its full range hits 90%, and the cap is 95% (90% against units).
- **Movement:** a moving shooter ×0.75, a moving target ×0.85. A soldier riding in a vehicle moves with it.
- **Veterancy:** +15% per rank: Veteran ×1.15, Elite ×1.30, Heroic ×1.45.
- **Examples** (regular, nobody moving): a Ranger at mid range hits infantry 42% of the time, a Crusader 66%, a building 95%. A Crusader's gun at mid range hits a Ranger 51% of the time and a Battlemaster 76%.
- **A miss** lands on the ground beside the target, 4 to 15 beyond its edge, or in the air beside a flying one. The shot goes there as a shot at the ground does: tracers aim there, a rocket or shell flies there and explodes, and its blast still hurts whatever it reaches. A missed bullet hurts nobody and kicks up a puff of dirt (`FX_CoHAccuracyMiss`), shown only where the local player can see.
- **Never rolls:** weapons whose damage type has no `Class` line (flame, poison, lasers, microwaves, hacking, melee), weapons listed in `NeverMisses` (the Avenger's point-defence lasers), weapons with a blast over `MaxDamageRadius` (8), weapons that scatter (artillery), weapons that go off where the shooter stands, shots at the ground, and a projectile going off. Mines, projectiles and props aren't rolled against.

## Skirmish AI

Nothing to change: the AI's shots roll by the same rules, so its veterans, its range and its movement matter the same way.

## Hit modifiers for other slices

`Accuracy.h` publishes the stack the chance is made with. Cover (05), Suppression (06) and Veterancy (12) add their own modifier in their `init`, without editing this slice:

```cpp
CoH::Accuracy::HitModifier inCover(const CoH::Accuracy::Shot &shot)
{
	return isInCover(shot.target) ? CoH::Accuracy::scaleChance(0.7f) : CoH::Accuracy::HitModifier();
}

void init(CoH::Feature &self)
{
	CoH::Accuracy::addHitModifier(self, "Cover", CoH::Accuracy::ModifierOrder::Cover, inCover);
}
```

- A modifier gets the `Shot`: shooter, target, weapon, distance and range, who is moving, the target's size class and radius, the shooter's rank. It answers `chance * multiply + add` (`scaleChance(m)`, `addToChance(points)`, chances being 0..1).
- **Order:** modifiers apply to the weapon's accuracy at that range by `ModifierOrder` (Movement 200, TargetSize 300, Cover 400, Suppression 500, Veterancy 600), then by name. Scaling commutes; the order matters for modifiers that add. The min and max apply once, at the end.
- A modifier counts only while its feature is on (and Accuracy is).
- **Standing in:** a modifier added under the name of a built-in one (`ShooterMoving`, `TargetMoving`, `TargetSize`, `Veterancy`) replaces it while its feature is on, whichever init ran first; the built-in one is back when that feature is off. Veterancy (12) can replace the +15% per rank this way.

## Settings (`CoHAccuracy` in [Data/INI/10_Accuracy.ini](Data/INI/10_Accuracy.ini))

| Setting | Default | Meaning |
|---|---|---|
| `Class = <DamageType> <near%> [<mid%> <far%>]` | see the pack | accuracy of weapons with that damage type at point-blank, half range and full range; one value for all three; types without a line never roll |
| `Weapon = <weapon> <near%> [<mid%> <far%>]` | see the pack | one weapon's own profile, over its class |
| `NeverMisses = <weapon> ...` | the Avenger's lasers | weapons that never roll |
| `MaxDamageRadius` | 8 | a weapon with a bigger blast is an area weapon and doesn't roll |
| `MinChance`, `MaxChance`, `MaxChanceBuilding` | 5%, 90%, 95% | the stacked chance is kept between the min and the max for the target |
| `SizePoint = <radius> <percent>` | 7 60%, 10 70%, 16 90%, 20 100%, 30 115%, 45 140% | the size multiplier by bounding radius, straight lines in between, flat outside |
| `BuildingSizeMin` | 180% | a building's size multiplier is at least this |
| `ShooterMoving`, `TargetMoving` | 75%, 85% | the movement multipliers |
| `MovingSpeed` | 0.3 | faster than this (world units a frame) is moving |
| `VeterancyPerRank` | 15% | added to the multiplier for each rank |
| `MissDistanceMin`, `MissDistanceMax` | 4, 15 | how far beyond the target's edge a miss lands |
| `MissFX` | `FX_CoHAccuracyMiss` | played where a missed bullet lands; the pack defines it with the retail `InfantryMortarDust` |

The block starts from the defaults: its `Class`, `Weapon`, `NeverMisses` and `SizePoint` lines are the whole list.

## Hooks

| Hub point | Used for |
|---|---|
| `weaponFiringAtObject` (`WeaponTemplate::fireWeaponTemplate`, new in this task) | the roll; a miss writes the spot and retargets the shot |
| `gameStartAfterMap`, `logicFrame`, `gameEnd` | the hit tally; once a minute it goes to the CoH log (`Accuracy: frame N, H hits of R rolls`) |

## Files

| File | What |
|---|---|
| `Accuracy.h` | public: `isActive`, `addHitModifier` |
| `Domain/Shot.h` | the `Shot` a modifier sees, `HitModifier`, `ModifierOrder` |
| `Domain/ModifierStack` | the named modifiers, their order, features on and off, standing in |
| `Domain/HitChance` | which weapons roll, accuracy by range, the size curve, the built-in modifiers, the clamps, the roll, the miss offset |
| `Engine/Config` | the `CoHAccuracy` block; a weapon's profile, looked up once per game |
| `Engine/Shots.cpp` | the hub subscriber: describes the shot, rolls, picks the miss spot |
| `UI/MissPuff.cpp` | the dirt puff |
| `Tests/` | the hit chance math, the modifier stack, 0% and 100% |

## Engine notes

- The hook sits in `WeaponTemplate::fireWeaponTemplate` after the range checks and before the fire effects. A miss turns the shot into a shot at a position, the way the engine's own "sneaky targeting offset" (`AIUpdateInterface::getSneakyTargetingOffset`, a few lines above) already does: `victimObj` null, `victimID` invalid, `victimPos` the spot. Everything after that (fire effects, scatter, lasers, delayed damage, projectiles, countermeasures) then handles it as a shot at the ground.
- `fireWeaponTemplate` is called a second time when a projectile goes off (`isProjectileDetonation`); the hook skips that call, so a shot rolls once.
- `dealDamageInternal` with no victim and a zero radius hurts nobody (`curVictim` is null), so a missed bullet does no damage; a weapon with a radius hits whatever is in it at the spot.
- `Weapon::privateFireWeapon` still sees the original victim, so reloads, clips, the AI's attack state and stealth reveal behave as they did.
- A weapon's profile is looked up once per weapon template per game (`Engine/Config.cpp`), not per shot.
- Target size is `GeometryInfo::getBoundingCircleRadius`: the radius for a cylinder or sphere, `sqrt(major² + minor²)` for a box. Retail: infantry 7 to 10, light vehicles 15 to 20, an Overlord 29, buildings 40 to 90 (a Gattling cannon 11).
- Rolls draw `GameLogicRandomValue(0, 9999)`, then the miss draws an angle and a spread with `GameLogicRandomValueReal`. The dirt puff uses only client effects. Nothing needs saving.
