# 10 Reinforce

**Branch** `coh/reinforce` · **Needs** 09, 01 · **Slice** `CoH/Features/Reinforce/` · **Flag** `Reinforce`

## Goal

Damaged squads are refilled rather than replaced, so veterans are kept.

## Behavior

- A squad below full strength can buy a new member for part of its cost while it is near a command center, barracks, or a forward reinforcement point in a held, connected sector.
- New members take the squad's veterancy rank.
- Transports such as the Humvee or Battle Bus can be reinforcement points if set in INI.
- The AI reinforces squads before it buys new ones.

## Done

Domain tests cover the cost, the range and the connected-sector rule. Reinforcing works in game and for the AI.
