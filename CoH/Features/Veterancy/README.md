# Veterancy

Task [12](../../Docs/Tasks/12-Veterancy.md) · Flag `Veterancy` · Public header [`Veterancy.h`](Veterancy.h)

Surviving units become much better, so losing a veteran hurts.

## Behavior

- **Ranks:** the engine's own: Veteran, Elite and Heroic, with its thresholds (`ExperienceRequired`), chevrons, promotion sounds and its own bonuses (more health, more damage, a heroic's self-repair, rank weapon sets). The pack adds CoH's bonuses on top.
- **Experience from damage:** dealing damage worth a victim's full health earns `DamageExperience` (50%) of its kill value. A kill still pays the whole value, as in the original game, so a unit that fights well ranks up even when another lands the last shot. It earns from what earns a kill: enemies of a playable side, not structures under construction. Fractions of a point add up over many small hits.
- **Bonuses per rank,** by class (infantry, vehicles, aircraft) and changed per unit with `Object` lines:

  | Bonus | Veteran / Elite / Heroic (infantry) | How |
  |---|---|---|
  | Accuracy | x1.15 / x1.30 / x1.50 | Accuracy's `Veterancy` hit modifier, in place of Accuracy's built-in +15% a rank |
  | Damage taken | x0.90 / x0.80 / x0.70 | after armor and the battle plan (hub `damageReceived`) |
  | Suppression resistance | 20% / 40% / 60% | for Suppression (06), through `Veterancy.h` |
  | Speed | x1.05 / x1.10 / x1.15 | the current locomotor's top speed (hub `unitSpeed`) |
  | Sight | x1.10 / x1.15 / x1.20 | vision and shroud clearing, scaled on each rank change |
  | Reload | x1.15 / x1.30 / x1.50 | rate of fire and reloads, in place of the engine's x1.2 / x1.4 / x1.6 for the rank (hub `weaponBonus`) |

  Vehicles and aircraft get less (see the pack); aircraft keep their speed.
- **A special at a rank:** `Upgrade` gives an object upgrade on reaching the rank, once. The pack gives an elite Humvee a battle drone and an elite Tomahawk a scout drone. An upgrade can unlock a weapon set or a command set, so an ability can come with a rank.
- **Squads share veterancy** (with Squads on): the members' experience is one pool. What any member earns, by damage or kills, goes to all of them by the end of the frame, and a new member (a reinforcement) joins at the squad's experience within a second. The member who earned the promotion plays it; the others rank up quietly.
- **Tooltip:** a unit's build button lists what each rank gives it, for trainable units: "Veteran: +15% accuracy, -10% damage taken, 20% suppression resistance, +5% speed, +10% sight, +15% fire rate".

## Skirmish AI

Nothing to change: the AI's units earn and use veterancy by the same rules.

## Tuning

The `CoHVeterancy` block in [`Data/INI/10_Veterancy.ini`](Data/INI/10_Veterancy.ini):

| Setting | Default | What |
|---|---|---|
| `DamageExperience` | 50% | share of a victim's kill value earned per full health of damage dealt |
| `SquadsShare` | Yes | a squad's members share one experience pool and rank |
| `Infantry`, `Vehicle`, `Aircraft` | see the pack | `= <rank> <bonus> <value> ...`: what the rank gives the class |
| `Object` | Humvee, Tomahawk | `= <object> <rank> <bonus> <value> ...`: one unit's changes to one rank, over its class |

Bonuses are `Accuracy`, `DamageTaken`, `SuppressionResistance`, `Speed`, `Sight`, `Reload` (percents) and `Upgrade` (an object upgrade's name). A bonus a line doesn't name keeps its value. Ranks are `REGULAR`, `VETERAN`, `ELITE`, `HEROIC`. A general's version (`AirF_AmericaVehicleHumvee`) takes its plain unit's lines and then its own; names match without case.

## How it plugs in

- **Accuracy:** `Accuracy::addHitModifier(self, "Veterancy", ModifierOrder::Veterancy, ...)`. A modifier under a built-in's name stands in for it while its feature is on, so with Veterancy on the rank's accuracy replaces Accuracy's +15% a rank and never stacks on it; with Veterancy off Accuracy's own is back. No change to Accuracy was needed.
- **Squads:** `Squads.h` only (`isActive`, `squadOf`, `members`).
- **Hub:** `damageDealt`, `killScored`, `weaponBonus`, `commandTooltip`, `logicFrame`, `objectCreated`, `objectDestroyed`, `gameEnd`, and four new points: `damageReceived`, `veterancyChanged`, `unitSpeed` (from two calls: `onLocomotorChosen`, `onLocomotorSpeed`).
- **Saved:** each unit's carried fraction of a point and each squad member's experience at the last share (`registerSaveData`, "Veterancy"). Sight is saved with the object.

## Public header

`Veterancy.h`: `isActive()`, `suppressionResistance(unit)` (0..1; Suppression 06 scales what it adds by `1 -` this), `accuracyMultiplier(unit)`, `damageTakenMultiplier(unit)` (for Cover 05 or a tooltip that wants the numbers). All answer "no bonus" while the flag is off.

## Engine notes

- Experience: `ExperienceTracker::addExperiencePoints` (`Z/GameLogic/Object/ExperienceTracker.cpp` 162) scales by the object's experience scalar and follows an experience sink (spawned units credit their master). `setExperienceAndLevel` sets both and doesn't scale. The level is found from `ThingTemplate::getExperienceRequired`.
- A kill: `Object::scoreTheKill` raises the hub's `killScored` and then adds the experience, so the squad share runs at the frame's end, not in the subscriber.
- The engine's rank bonuses: `Object::onVeterancyLevelChanged` (`Object.cpp` 3172) sets the rank's weapon bonus condition (`VETERAN`, `ELITE`, `HERO`: GameData's `WeaponBonus` lines, rate of fire x1.2/x1.4/x1.6 and damage x1.1/x1.2/x1.3) and weapon set flag, gives a veterancy upgrade (`UpgradeCenter::findVeterancyUpgrade`), and `ActiveBody::onVeterancyLevelChanged` scales max health by `HealthBonus_Veteran/Elite/Heroic` (x1.2/x1.3/x1.5, keeping the health ratio). The hook goes after all of that, before the promotion effects.
- Bonuses add in `Weapon::computeBonus` (field = 1 + each share), so a rank's share is read back with `WeaponBonusSet::appendBonuses(1 << condition, ...)`, GameData's and the weapon's extra set. The reload swap scales the total by ours / engine's, which commutes with FireRate's division.
- Damage: `ActiveBody::attemptDamage` applies the armor, then `m_damageScalar` (the defensive battle plan) only when `allowModifier` (not status, kill-pilot or subdual damage) and not for `DAMAGE_UNRESISTABLE`. `damageReceived` is raised right there.
- Speed: a `Locomotor` doesn't know its object, and its `m_maxSpeed` only caps the template's speed. `AIUpdateInterface::chooseGoodLocomotorFromCurrentSet` (every `doLocomotor`, and on a set change) tells the hub whose current locomotor it is; `Locomotor::getMaxSpeedForCondition` (every move mode, group speed, `getCurLocomotorSpeed`) asks the hub. The hub counts an entry only while that object is alive and the locomotor is still its current one. Loaded games re-register on the first move.
- Sight: `Object::setVisionRange`, `setShroudClearingRange`, scaled by ratio as the battle plan does (`BattlePlanUpdate.cpp` 770), so others' changes stay.
- Drones: `ObjectCreationUpgrade` modules with `ConflictsWith` (the Humvee's three drones) skip a drone the object can't have with the one it has.

## Testing

- Unit tests: `z_googletest --gtest_filter=Veterancy*` (ranks and tables, experience from damage, the squad pool, swapping the engine's rate of fire, combining with Accuracy's modifier stack).
- Test folder `C:\Users\ibm21\Games\ZH-Veterancy` (retail hardlinked, own exe): `install.ps1 -GameDir`, `skirmish.ps1 -GameDir`, `replaycheck.ps1 -GameDir`.
- Flags-on replay: `Replays\CoHVeterancy\veterancy-allflags-usa-china-desert-15min.rep` (Sectors, GeneralsArsenal, Upkeep, Squads, Accuracy, FireRate, Pacing, Veterancy; Hard AIs, America against China) plays clean. In it 123 promotions: squads ranking up together, an elite Humvee getting its battle drone, an elite Tomahawk its scout drone.
- `-cohLog` logs each rank change, `Veterancy: <unit> <id> rank <from> to <to> (experience <n>)`, and each upgrade a rank gives: `Veterancy: <unit> <id> gained <upgrade>`.
