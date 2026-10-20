# 20 Vehicle movement

**Branch** `coh/vehicle-movement` · **Needs** 00, K1 · **Pairs with** 07, 18, 19 · **Slice** `CoH/Features/VehicleMovement/` · **Flag** `VehicleMovement`

## Goal

Vehicles that drive like what they are: heavy tanks slow to start, slow to stop and slow to turn, light vehicles quick, armor and riders adding weight. Later, tanks that back up instead of turning their thin rear to the enemy.

The user's words: "an overhaul of the vehicle movement system perhaps? with weights so heavier tanks are slower, smaller vehicles are faster? armour affects things too, like upgrading to composite armour adds weight". Earlier: "it adds reversing for short movements".

Two phases. Phase 1 merges before Phase 2 starts.

## Phase 1: weight

### Behavior

- Every ground vehicle has a **weight** in tons. A `Unit` line in the pack gives it a class (`Light`, `Medium`, `Heavy`, `SuperHeavy`, each with its tons) or its own tons. A vehicle no line names weighs the `Medium` class, which handles as in retail.
- Helicopters, jets, hover vehicles and infantry are left alone.
- **Upgrades add weight** (`Upgrade` lines): Composite Armor, the Overlord's and Emperor's riders (gattling, bunker, speaker tower), and so the loadouts the Tank loadouts slice (18) fits at creation, since it gives the same upgrades. An object upgrade counts once the vehicle has it; a player upgrade (Composite Armor) counts for vehicles that react to it. Movement follows at once when an upgrade completes.
- **Weight changes handling** through five curves of `weight → percent`, straight lines between their points:
  top speed, acceleration, braking, turn rate, and the speed lost going uphill (per 100% grade, never below a floor).
- The retail numbers stay the base: a 30-ton vehicle with no upgrades accelerates, brakes and turns as in retail; the curves make lighter ones quicker and heavier ones slower. After the first watch test (2026-10-19) tanks' top speed is a little lower than retail: 100% at 18 t, 94% at 30 t.
- **Ground and roads** (the user, after the first watch test: "can we also do a road type speed? road vs dirt vs rocks vs. whatever?"): top speed by surface and drive. Paved roads and tiles 120% wheels / 105% treads, dirt tracks 110/102, open ground 100, beach sand 85/95, rock 80/90, snow 90/95, deep snow 75/85 (agreed 2026-10-19). Surfaces come from the map's tile textures and its road objects, read once per map. Pathfinding doesn't seek roads.
- The skirmish AI's vehicles follow the same rules; nothing for it to learn.

### Order with other slices

All multiply, so the order changes nothing but is written down:
1. the locomotor's retail speed and its damaged speed; the engine's caps (`setMaxSpeed`, a group's speed);
2. hub `unitSpeed`: Veterancy's rank speed, then VehicleMovement's weight and slope (feature order);
3. Retreat's bonus (11), on its own hub point after `getMaxSpeedForCondition`, when merged.

Pacing changes prices and build times only; UnitScale changes sizes only.

### Hook points

- Top speed, slope and ground: hub `unitSpeed` (`Locomotor::getMaxSpeedForCondition`), already there for Veterancy.
- Acceleration, braking and turn rate: one new general point `unitHandling(object, Handling, value)`, called from `Locomotor::getMaxAcceleration`, `getBraking` and `getMaxTurnRate`. These are what every move mode (treads, wheels, legs, thrust) reads.
- Not `AIUpdateInterface::chooseLocomotorSetExplicit`: the old Economy slice capped speed there with `setMaxSpeed`, but the locomotor's `m_max*` fields only ever lower the template's values (lights couldn't get faster) and are saved with the object, so flag off after a save would no longer be original.

### Done

- Domain tests for the curves and the slope rule.
- `replaycheck.ps1` passes with the flag off and on (a fresh flag-on replay).
- The user has watched a windowed skirmish at normal speed with their flags plus VehicleMovement: heavies feel heavy, lights feel quick.
- README, hooks table, CONTEXT.md updated. Merged after the user's OK.

## Phase 2: reversing

- A short move to a spot behind a vehicle (within a distance and an angle from INI) drives there in reverse at reduced speed instead of turning round.
- A **reverse-move command** (button and hotkey) backs away to a spot with the hull keeping its facing; with Directional armor (07) the front is the strong side.
- The skirmish AI backs damaged vehicles away from the enemy.
- The hook in `Locomotor.cpp`'s driving code stays minimal and goes through the hub. Treads and wheels may differ; hover and aircraft are skipped.
- Done: the same checks, plus an in-game self-test that a vehicle reverses to a point behind it.
