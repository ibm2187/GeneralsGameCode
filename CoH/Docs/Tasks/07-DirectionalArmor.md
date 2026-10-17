# 07 Directional armor

**Branch** `coh/directional-armor` · **Needs** 00 · **Slice** `CoH/Features/DirectionalArmor/` · **Flag** `DirectionalArmor`

## Goal

Flanking tanks pays. Facing the enemy with your strongest armor matters.

## Behavior

- Vehicles have front, side and rear armor values. Weapons have a penetration value at near and far range.
- On a hit, the angle between the shot and the vehicle's facing picks the armor side.
- The chance to penetrate compares penetration and armor, using synced randomness.
- A shot that doesn't penetrate is deflected and deals little or no damage, with a spark effect and an audio cue.
- Infantry and buildings are unaffected.
- The AI attacks vehicles from the side and rear when it can.

## Likely hook points

Damage dealt (the general point "before damage is applied to a body").

## Done

Domain tests cover the angle-to-side mapping and the penetration odds. Deflections are visible in game.
