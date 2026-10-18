# 03b Pacing

**Branch** `coh/pacing` · **Needs** 01, 03 · **Slice** `CoH/Features/Pacing/` · **Flag** `Pacing`

## Goal

Fewer, more valuable units, CoH style. With Sectors, Upkeep and the other packs on, the skirmish AI out-builds a human: sectors pay far more than retail supplies while units keep retail prices and build times. Slow the economy down so armies stay small, and keep the AI a fair challenge.

## Behavior

- Cost and build-time multipliers by unit class (infantry, vehicle, aircraft, structure) and for single objects, in INI. A general's version of a unit (`Tank_ChinaTankOverlord`) uses the plain unit's line unless it has its own.
- Dozers, workers, harvesters and drones keep retail cost and build time.
- An AI handicap by difficulty (Easy, Medium, Hard): a multiplier on the AI's income and on its build times.
- Sectors' income and Upkeep's cap and steps are retuned in their INI packs (no code change), each number explained in the slice README.
- Measured first and after: AI-vs-AI observer games (`-cohSide Observer -cohOpponents 2 -cohAI hard`) on two retail maps, with the user's flags and with every flag off (retail). Income per minute, army size and units built at minutes 5, 10 and 15, in the README.

## Hook points

The hub's `buildCost` and `buildTime` (`ThingTemplate::calcCostToBuild`, `calcTimeToBuild`) and `income` (`Money::deposit`). No new points in original code. The Kernel's `-cohStats` option logs the measurements.

## Done

- Domain tests pass. `install.ps1` works. `replaycheck.ps1` passes with the flags off and on.
- The AI's army at minute 10 is about half of what it was, and a human who expands normally keeps up.
- The before and after measurements are in the README.
- The user has played a windowed skirmish against a Hard AI and finds it fair.
- The READMEs, the hooks table and CONTEXT.md are updated.
