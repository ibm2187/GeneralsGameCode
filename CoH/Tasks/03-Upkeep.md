# 03 Upkeep and population

**Branch** `coh/upkeep` · **Needs** 01 · **Slice** `CoH/Features/Upkeep/` · **Flag** `Upkeep`

## Goal

A big army costs money to keep. Fewer, better units pay off; spamming units doesn't.

## Behavior

- Each unit has a population cost. The cap starts low and rises with buildings and with sectors held.
- Upkeep takes a share of sector income that grows with population in steps (for example 0%, 10%, 25%, 40%). The steps are set in INI.
- The money tooltip shows gross income, upkeep and net income.
- The AI keeps its army size where its net income stays positive.

## Done

Domain tests cover the upkeep steps and the cap. Upkeep is visible in game. The AI doesn't stall its economy.
