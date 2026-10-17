# 04 Accuracy

**Branch** `coh/accuracy` · **Needs** 00 · **Slice** `CoH/Features/Accuracy/` · **Flag** `Accuracy`

## Goal

Direct fire can miss. Skill, range and movement decide the fight more than unit count.

## Behavior

- Each direct-fire shot rolls to hit, using synced randomness.
- The chance starts from the weapon's base accuracy, then changes with:
  - range (near, mid, far)
  - the shooter moving
  - the target moving
  - target size (infantry, vehicle, building)
  - veterancy, about +15% per rank
- Modifiers from other slices plug in through a published modifier interface: Cover, Suppression and Veterancy add their own.
- A miss hits the ground near the target (splash still applies there) and shows a dirt puff. Artillery and area weapons don't roll.
- Base accuracy is set per weapon in INI, with sensible defaults by weapon class.

## Likely hook points

Weapon fire (the general point "before a weapon fires at a victim").

## Done

Domain tests cover the hit-chance math and the modifier stack. Misses are visible in game. Retail replays still play with the flag off.
