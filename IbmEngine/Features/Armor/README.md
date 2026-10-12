# Armor

Vehicles have front, side and rear armor and weapons have penetration, like in Company of Heroes. Feature flag: `Armor`.

> A direct hit on a vehicle penetrates with chance = penetration ÷ armor of the side it hits (always, when penetration is at least the armor).
> A shot that doesn't penetrate is **deflected**: it does 10% of its damage and throws sparks; anti-tank shots also float "Deflected" over the vehicle.

- **Side hit:** where the shooter (or its rocket or shell) is, seen from the vehicle's hull: a 90° front arc, a 90° rear arc, and the sides between. Flanking and hitting the rear pay off; a tank facing the enemy shrugs off far more.
- **Only direct hits roll.** Area damage always gets through: artillery, bombs and super weapons (damage radius over 10), and splash on anything but the target.
- **Weapons that never roll:** any whose DamageType has no `Penetration` line and that have no `Weapon` line: flame, poison, radiation, microwaves, crushing, Jarmen Kell's pilot snipe.
- **Aircraft have no armor** (`Exempt = AIRCRAFT`); buildings and infantry don't either.
- Rolls use the game logic's random numbers, so all PCs and replays agree.

| Weapon kind (DamageType) | Penetration |
|---|---|
| Rifles (SMALL_ARMS) | 5 |
| Snipers | 10 |
| Machine guns (GATTLING) | 20 |
| Comanche cannon | 30 |
| Rockets and small shells (EXPLOSION) | 70 |
| Tank guns (ARMOR_PIERCING) | 100; Scorpion 70–80, Crusader 110, Paladin 120, Marauder 100–120 with salvage, Overlord 140 |
| Anti-tank infantry (INFANTRY_MISSILE) | 110; laser-guided Missile Defender 150 |
| Lasers | 120 |
| Jet missiles | 140 |

| Armor (front / side / rear) | Units |
|---|---|
| 10–30 | Combat Bikes, Technical, Rocket Buggy, Radar Van, Humvee, Ambulance, Toxin Tractor, Bomb Truck |
| 40–70 | Scud Launcher, Tomahawk, Inferno, Listening Outpost, Troop Crawler, Quad Cannon, Battle Bus, Nuke Cannon; any other vehicle 50 / 40 / 30 |
| 80–100 | Gattling, ECM, Dragon, Scorpion, Microwave |
| 110–140 | Battlemaster, Avenger, Crusader, Marauder, Paladin |
| 180 / 130 / 90 | Overlord, Emperor |

So a Crusader's gun (110) always gets through a Battlemaster's front (110), a Battlemaster's gun (100) gets through a Crusader's front 83% of the time, and a rifle (5) gets through a Crusader's front 4% of the time.

## The IbmArmor block

| Line | Meaning |
|---|---|
| `DeflectedDamage = 10%` | Share of a deflected shot's damage that still gets through |
| `FrontArc = 90`, `RearArc = 90` | Full widths of the front and rear arcs, in degrees |
| `DirectMaxRadius = 10` | Weapons with a bigger damage radius are area weapons and never roll |
| `DeflectTextPenetration = 50` | "Deflected" floats over the vehicle for weapons with at least this penetration (0: never) |
| `DeflectFX = FX_IbmArmorDeflect` | Effect where a deflected shot hits (the pack defines it: retail `TankStruckSparks`) |
| `Penetration = <DamageType> <value>` | Penetration of a whole kind of weapon |
| `Weapon = <Weapon> <value>` | Penetration of one weapon, and of the generals' versions of it (`Nuke_OverlordTankGun`) |
| `Kind = <KindOf> <front> <side> <rear>` | Armor of units with no line of their own; the first match wins |
| `Exempt = <KindOf> ...` | Units with any of these get no `Kind` armor |
| `Unit = <Object> <front> <side> <rear>` | Armor of one unit, and of the generals' versions of it (`Tank_ChinaTankEmperor` has its own) |

## Files

| File | What it does |
|---|---|
| `Armor.cpp`, `Armor.h` | The penetration roll and the `IbmArmor` INI block |
| `Data/INI/Features/Armor/10_Armor.ini` | Every number above, and the deflection effect |

Hooks: `WeaponTemplate::dealDamageInternal` (Weapon.cpp) multiplies each victim's damage by the roll's result, next to Economy's damage against infantry; INI.cpp registers the `IbmArmor` block.

## Testing

The Demo flag with Economy on places one of each tank and an enemy tank group; hit a tank from the front and from behind.
