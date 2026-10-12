# Accuracy

Troops aren't 100% accurate, and veterancy makes them better shots. Feature flag: `Accuracy`.

With the flag on, every shot from a direct-fire weapon at a unit rolls to hit:

> hit chance = base accuracy × veterancy × range × movement × cover, kept between 5% and 95%

| Part | Default |
|---|---|
| Base accuracy by weapon kind | rifles 65%, machine guns 50%, snipers 95%, tank guns 75%, rockets and missiles 80% |
| Veterancy | Veteran ×1.15, Elite ×1.30, Heroic ×1.45 |
| Range | full accuracy up to half the weapon's range, falling to ×0.75 at full range |
| Shooter moving | ×0.80 (a soldier riding in a vehicle moves with it) |
| Target moving | ×0.85 |
| Target in cover | ×0.70 for infantry within 25 of a building or bush |

- A miss lands on the ground beside the target, up to 15 beyond its edge. Bullets then hurt nobody; rockets and shells explode there, so their blast can still catch someone.
- Buildings are never missed, and only infantry, vehicles and aircraft are rolled against.
- Weapons that never miss: flame, poison, lasers, artillery (blast radius over 10), bombs, melee and anything else not listed by kind.
- The weapon kind comes from its `DamageType`; `Weapon = <name> <percent>` lines in the pack give single weapons their own base (100% = never misses).
- Rolls use the game logic's random numbers, so all PCs and replays agree.

## Files

| File | What it does |
|---|---|
| `Accuracy.cpp`, `Accuracy.h` | The hit roll and the `IbmAccuracy` INI block |
| `Data/INI/Features/Accuracy/10_Accuracy.ini` | All the numbers above |

Hooks: `WeaponTemplate::fireWeaponTemplate` (Weapon.cpp) rolls before a shot is resolved; INI.cpp registers the `IbmAccuracy` block.

## Testing

The debug build logs the running hit rate every 500 rolls (`IbmEngine accuracy:` in `DebugLogFileD.txt`).
