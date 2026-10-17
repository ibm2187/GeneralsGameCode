# 12 Veterancy

**Branch** `coh/veterancy` · **Needs** 00 · **Slice** `CoH/Features/Veterancy/` · **Flag** `Veterancy`

## Goal

Surviving units become much better, so losing a veteran hurts.

## Behavior

- The engine's existing ranks (Veteran, Elite, Heroic) are used as CoH's three ranks.
- Experience comes from damage dealt and kills, not only kills.
- Bonuses are set per unit type in INI: accuracy, received damage, suppression resistance, speed, sight, reload.
  Some units get a special bonus at a rank, such as a new ability.
- The tooltip shows the rank bonuses.
- Squad veterancy is shared by the squad.

## Done

Domain tests cover the experience and bonus tables. Bonuses take effect in game.
