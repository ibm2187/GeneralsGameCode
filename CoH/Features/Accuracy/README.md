# Accuracy

**Flag** `Accuracy` · **Task** [04](../../Docs/Tasks/04-Accuracy.md) · **Public header** [Accuracy.h](Accuracy.h)

Direct fire can miss. Range, movement, target size and veterancy decide the fight more than unit count does.

## How it plays

- Every shot of a direct-fire weapon at an object rolls to hit:
  > hit chance = base × range × shooter moving × target moving × target size × veterancy (× other slices' modifiers), kept between 5% and 95%
- **Base accuracy** comes from the weapon's `DamageType` (`Class` lines): rifles and light machine guns 70%, gattlings 60%, snipers 95%, tank guns 80%, rockets, missiles and small shells 75%, jet missiles 85%. `Weapon` lines give single weapons their own base: heavy machine guns with `SMALL_ARMS` damage 60%, Burton's rifle 95%.
- **Range:** ×1.15 up to a third of the weapon's range, ×1 in the middle third, ×0.85 in the last third.
- **Movement:** a moving shooter ×0.75, a moving target ×0.85. A soldier riding in a vehicle moves with it.
- **Target size:** infantry ×0.9, vehicles ×1, flying aircraft ×0.9, buildings ×1.5 (so 95% for nearly every weapon).
- **Veterancy:** +15% per rank: Veteran ×1.15, Elite ×1.30, Heroic ×1.45.
- **A miss** lands on the ground beside the target, 4 to 15 beyond its edge, or in the air beside a flying one. The shot goes there as a shot at the ground does: tracers aim there, a rocket or shell flies there and explodes, and its blast still hurts whatever it reaches. A missed bullet hurts nobody and kicks up a puff of dirt (`FX_CoHAccuracyMiss`), shown only where the local player can see.
- **Never rolls:** weapons whose damage type has no `Class` line (flame, poison, lasers, microwaves, hacking, melee), weapons with a blast over `MaxDamageRadius` (8), weapons that scatter (artillery), weapons that go off where the shooter stands, shots at the ground, and a projectile going off. Mines, projectiles and props aren't rolled against.

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

- A modifier gets the `Shot`: shooter, target, weapon, distance and range, who is moving, the target's size, the shooter's rank. It answers `chance * multiply + add` (`scaleChance(m)`, `addToChance(points)`, chances being 0..1).
- **Order:** modifiers apply by `ModifierOrder` (Range 100, Movement 200, TargetSize 300, Cover 400, Suppression 500, Veterancy 600), then by name. Scaling commutes; the order matters for modifiers that add. The min and max apply once, at the end.
- A modifier counts only while its feature is on (and Accuracy is).
- **Standing in:** a modifier added under the name of a built-in one (`Range`, `ShooterMoving`, `TargetMoving`, `TargetSize`, `Veterancy`) replaces it while its feature is on, whichever init ran first; the built-in one is back when that feature is off. Veterancy (12) can replace the +15% per rank this way.

## Settings (`CoHAccuracy` in [Data/INI/10_Accuracy.ini](Data/INI/10_Accuracy.ini))

| Setting | Default | Meaning |
|---|---|---|
| `Class = <DamageType> <percent>` | see the pack | base accuracy of weapons with that damage type; types without a line never roll |
| `Weapon = <weapon> <percent>` | see the pack | one weapon's base accuracy, over its class; 100% never misses |
| `MaxDamageRadius` | 8 | a weapon with a bigger blast is an area weapon and doesn't roll |
| `MinChance`, `MaxChance` | 5%, 95% | the stacked chance is kept between these |
| `NearRange`, `FarRange` | 33%, 67% | the range bands, as a share of the weapon's range |
| `Near`, `Mid`, `Far` | 115%, 100%, 85% | the range multipliers |
| `ShooterMoving`, `TargetMoving` | 75%, 85% | the movement multipliers |
| `MovingSpeed` | 0.3 | faster than this (world units a frame) is moving |
| `TargetSize = <Infantry\|Vehicle\|Aircraft\|Building> <percent>` | 90, 100, 90, 150 | the size multipliers |
| `VeterancyPerRank` | 15% | added to the multiplier for each rank |
| `MissDistanceMin`, `MissDistanceMax` | 4, 15 | how far beyond the target's edge a miss lands |
| `MissFX` | `FX_CoHAccuracyMiss` | played where a missed bullet lands; the pack defines it with the retail `InfantryMortarDust` |

The block starts from the defaults: its `Class` and `Weapon` lines are the whole list.

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
| `Domain/HitChance` | which weapons roll, the built-in modifiers, the clamp, the roll, the miss offset |
| `Engine/Config` | the `CoHAccuracy` block; a weapon's base, looked up once per game |
| `Engine/Shots.cpp` | the hub subscriber: describes the shot, rolls, picks the miss spot |
| `UI/MissPuff.cpp` | the dirt puff |
| `Tests/` | the hit chance math, the modifier stack, 0% and 100% |

## Engine notes

- The hook sits in `WeaponTemplate::fireWeaponTemplate` after the range checks and before the fire effects. A miss turns the shot into a shot at a position, the way the engine's own "sneaky targeting offset" (`AIUpdateInterface::getSneakyTargetingOffset`, a few lines above) already does: `victimObj` null, `victimID` invalid, `victimPos` the spot. Everything after that (fire effects, scatter, lasers, delayed damage, projectiles, countermeasures) then handles it as a shot at the ground.
- `fireWeaponTemplate` is called a second time when a projectile goes off (`isProjectileDetonation`); the hook skips that call, so a shot rolls once.
- `dealDamageInternal` with no victim and a zero radius hurts nobody (`curVictim` is null), so a missed bullet does no damage; a weapon with a radius hits whatever is in it at the spot.
- `Weapon::privateFireWeapon` still sees the original victim, so reloads, clips, the AI's attack state and stealth reveal behave as they did.
- The base accuracy is looked up once per weapon template per game (`Engine/Config.cpp`), not per shot.
- Rolls draw `GameLogicRandomValue(0, 9999)`, then the miss draws an angle and a spread with `GameLogicRandomValueReal`. The dirt puff uses only client effects. Nothing needs saving.
