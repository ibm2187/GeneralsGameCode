# 11 Retreat

**Branch** `coh/retreat` · **Needs** 09 · **Slice** `CoH/Features/Retreat/` · **Flag** `Retreat`

## Goal

Units can be saved. A losing fight ends in retreat, not a wipe.

## Behavior

- A retreat button and hotkey send the unit to the nearest retreat point: the command center or a forward point.
- While retreating the unit moves faster and takes less damage. It ignores suppression and pinning and doesn't fire.
- At the retreat point, squads heal over time.
- The AI retreats squads and vehicles at low health.

## Done

Domain tests cover choosing the retreat point and the retreat modifiers. Retreat works for players and the AI.
