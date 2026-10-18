# 04b Fire rate

**Branch** `coh/fire-rate` · **Needs** 00 · **Pairs with** 04 · **Slice** `CoH/Features/FireRate/` · **Flag** `FireRate`

## Goal

Each shot matters. Zero Hour's rifles and machine guns fire 3 to 10 times a second, so Accuracy's rolls average out and a miss changes nothing. Fire about as slowly as Company of Heroes, with bigger hits.

## Behavior

- A weapon faster than its class's limit (seconds a shot, by damage type) is slowed to it; its damage rises by the same factor, so damage per second stays the same.
- Limits agreed with the user (2026-10-17): rifles one shot every 2 s, machine guns 1 s, tank guns 5 s, rockets and missiles 3 s; snipers and jets unchanged; at most ×8.
- The rate is averaged over a clip and its reload, and counted fully spun up for a gattling.
- Area weapons, artillery that scatters and weapons that go off where the shooter stands aren't slowed by their class. Single weapons can have their own limit, or none, in INI.
- Its own slice and flag, so it can be tried with Accuracy on or off.

## Hook points

`Weapon::computeBonus` (the hub's `weaponBonus`): the `RATE_OF_FIRE` and `DAMAGE` bonus fields.

## Done

Domain tests cover the fire cycle and the slowdown. Replays play clean with the flag off and on. The user has watched a skirmish with it.
