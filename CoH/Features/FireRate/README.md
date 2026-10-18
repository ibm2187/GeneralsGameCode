# FireRate

**Flag** `FireRate` · **Task** [04b](../../Docs/Tasks/04b-FireRate.md) · **Pairs with** [Accuracy](../Accuracy/README.md) · **Public header** [FireRate.h](FireRate.h)

Fewer, bigger shots. Zero Hour's rifles and machine guns fire 3 to 10 times a second, so with [Accuracy](../Accuracy/README.md) on, many rolls a second average out and a miss changes nothing. With FireRate on, fast weapons fire about as often as in Company of Heroes and hit harder by the same factor: damage per second stays the same, but every shot, and every hit roll, counts.

## How it plays

- A weapon that fires faster than its class allows is slowed to that limit (`Class` lines, by `DamageType`, in seconds a shot), and each shot does more damage by the same factor:
  - **Rifles** (`SMALL_ARMS`): one shot every 2 s. Ranger and Rebel ×6.7 (33 damage a shot), Red Guard ×2 (30).
  - **Machine guns** (`GATTLING`, `COMANCHE_VULCAN`, and the heavy machine guns with `SMALL_ARMS` damage, by `Weapon` lines): one a second. Humvee gun ×5 (50), Comanche gun and the upgraded Quad Cannon ×8. A gattling is held to the limit once fully spun up: the Gattling tank (2.5 shots a second, 7.5 spun up) ×7.5, 112 a shot, starting at one shot every 3 s and reaching one a second as it spins.
  - **Tank guns** (`ARMOR_PIERCING`): one every 5 s. Crusader and Battlemaster ×2.5 (150), Scorpion ×5, Overlord ×4.4.
  - **Rockets and missiles** (`INFANTRY_MISSILE`, `EXPLOSION`): one every 3 s. Tank Hunter and Missile Defender ×3 (120), Patriot ×4.4.
  - **Unchanged:** weapons already slower than their limit, snipers, jet missiles, flame, poison, lasers, microwaves, area weapons (a blast over `MaxDamageRadius`), artillery that scatters, and weapons that go off where the shooter stands.
- The rate counted is the average over a clip and its reload: a Ranger's burst of 3 every 0.9 s is 3.3 shots a second. The clip's shots and the reload stretch alike, so a burst stays a burst, just slower.
- A weapon is slowed at most `MaxSlowDown` (×8) times, so its shots don't grow far past what kills a target.
- Veterancy, upgrades and frenzy still speed the weapon up on top, by the same share as before. A gattling's spin-up (`CONTINUOUS_FIRE_FAST`, ×3 in retail) is counted in its rate, so a gattling spun up fires at the limit, and slower while it spins up.
- **What it does to a fight** (Accuracy's hit chances at mid range, nobody moving; the spread is the 10th to 90th percentile time to kill, simulated):

  | Fight | Retail | FireRate |
  |---|---|---|
  | Ranger vs Red Guard (42%) | 24 hits, 14-20 s | 33 a shot, 4 hits, 8-26 s |
  | Red Guard vs Ranger (42%) | 12 hits, 20-36 s | 30 a shot, 6 hits, 16-38 s |
  | Humvee gun vs Rebel (45%) | 12 hits, 4-7 s | 50 a shot, 3 hits, 3-10 s |
  | Crusader vs Battlemaster (76%) | 7 hits, 12-20 s | 150 a shot, 3 hits, 10-20 s |
  | Tank Hunter vs Crusader (76%) | 12 hits, 12-18 s | 120 a shot, 4 hits, 9-18 s |

  The average time to kill barely moves; the spread about doubles, so a run of hits or misses decides a fight. Bigger shots waste some damage on a nearly dead target, so a weapon far over its target's health kills a little slower (the Humvee gun about 10%).

## Skirmish AI

Nothing to change: its weapons follow the same rule. Its damage estimates ask the same bonuses, so it sees the bigger shots.

## Settings (`CoHFireRate` in [Data/INI/10_FireRate.ini](Data/INI/10_FireRate.ini))

| Setting | Default | Meaning |
|---|---|---|
| `Class = <DamageType> <seconds>` | see the pack | weapons with that damage type fire at most one shot every this many seconds; types without a line are never slowed |
| `Weapon = <weapon> <seconds>` | see the pack | one weapon's own limit, over its class; 0 never slows it. A `Weapon` line applies even to an area weapon |
| `MaxDamageRadius` | 8 | a weapon with a bigger blast is an area weapon and isn't slowed by its class |
| `MaxSlowDown` | 8 | a weapon is slowed at most this many times |
| `DamagePerSecond` | 100% | the share of its damage per second a slowed weapon keeps; a shot never does less than it did |

The block starts from the defaults: its `Class` and `Weapon` lines are the whole list.

## Hooks

| Hub point | Used for |
|---|---|
| `weaponBonus` (`Weapon::computeBonus`, new in this task) | divides the rate of fire and multiplies the damage by the weapon's slowdown |

## Files

| File | What |
|---|---|
| `FireRate.h` | public: `isActive`, `slowDown(weapon)` |
| `Domain/Slowdown` | a weapon's shots a second over its clip and reload and fully spun up, which limit holds it, the slowdown and the damage |
| `Engine/Config` | the `CoHFireRate` block; a weapon's slowdown, worked out once per game (and logged with `-cohLog`) |
| `Engine/FireRate.cpp` | the slice, the hub subscriber, the public answers |
| `Tests/` | the fire cycle, spin-up, the limits and exceptions, the slowdown, damage per second kept |

## Engine notes

- `Weapon::computeBonus` sums every bonus a weapon fires with (veterancy, upgrades, garrison and container bonuses, the weapon's own extra bonus). It is asked for each shot (`privateFireWeapon`), each reload (`reloadWithBonus`, `onWeaponBonusChange`), the reload time the UI and jets read (`Weapon::getClipReloadTime`), a projectile going off (`WeaponStore::handleProjectileDetonation`: the source is then the projectile, and the weapon the launcher's), and the AI's estimates. So one change there reaches all of them.
- `WeaponTemplate::getDelayBetweenShots` and `getClipReloadTime` divide by the `RATE_OF_FIRE` field; `getPrimaryDamage` and `getSecondaryDamage` multiply by `DAMAGE`. Bonuses add to these fields (`WeaponBonus::appendBonuses`); the slice multiplies the sum, so other bonuses keep their share.
- `getDelayBetweenShots` draws a synced random number when the weapon has a `Min`/`Max` delay, so the slice reads the delays with the getters `getMinDelayBetweenShots` and `getMaxDelayBetweenShots` (added for this) instead of calling it.
- After a clip's last shot the weapon waits for its reload instead of the delay: a clip of N fires N shots in (N - 1) delays plus one reload.
- A gattling's spin-up is a `CONTINUOUS_FIRE_MEAN`/`FAST` rate-of-fire bonus in the weapon's own bonus set (`WeaponTemplate::getExtraBonus`); the slice reads the `FAST` one with `WeaponBonusSet::appendBonuses`.
- `PreAttackDelay` (snipers, some missiles) isn't counted in the rate and isn't stretched.
- A weapon's slowdown is worked out once per weapon template per game, from its INI data alone, so it is the same on every machine. Nothing needs saving.
