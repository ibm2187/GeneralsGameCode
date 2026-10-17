# 13 Abilities

**Branch** `coh/abilities` · **Needs** 09 · **Slice** `CoH/Features/Abilities/` · **Flag** `Abilities`

## Goal

Squads have active tools, so micro decides fights.

## Behavior

- Squad abilities: grenade, satchel or sticky bomb, smoke, sprint, and anti-tank rockets for faction squads.
- Each has a cost, a cooldown, a range and a target type, set in INI.
- Some are unlocked by an upgrade at the barracks, and some by veterancy.
- Smoke blocks sight and drops accuracy through it, using an Accuracy modifier.
- The AI uses grenades on garrisons and units in cover, and anti-tank weapons on vehicles.

## Done

Domain tests cover cost and cooldown rules. Each ability works for players and the AI.
