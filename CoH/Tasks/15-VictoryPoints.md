# 15 Victory points

**Branch** `coh/victory-points` · **Needs** 01 · **Slice** `CoH/Features/VictoryPoints/` · **Flag** `VictoryPoints`

## Goal

An optional game mode where holding the map wins, not only destroying bases.

## Behavior

- A few central sectors become victory points.
- Each side starts with tickets. Holding more victory points than the enemy drains their tickets. A side at zero loses.
- Ticket bars are shown at the top of the screen.
- The mode is picked in the skirmish options.
- The AI values victory points above other sectors.

## Done

Domain tests cover the ticket drain. A match can be won on tickets.
