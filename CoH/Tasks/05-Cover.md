# 05 Cover

**Branch** `coh/cover` · **Needs** 04 · **Slice** `CoH/Features/Cover/` · **Flag** `Cover`

## Goal

Where infantry stands matters.

## Behavior

- Cover is found from nearby objects and terrain:
  - **Heavy cover:** walls, rubble, sandbags, building edges.
  - **Light cover:** fences, trees, wrecks, craters.
  - **Open ground.**
  - **Negative cover:** roads and water.
- Infantry in cover takes less damage and is harder to hit, adding to the Accuracy modifiers.
- Infantry in negative cover takes more.
- Cover is directional: it only helps against fire from the far side of the cover object.
- Explosions leave craters that become light cover.
- The cursor shows the cover at the move destination (green, yellow or red shields), as in CoH. Units show their current cover.
- The AI prefers cover when it moves infantry to attack or hold.

## Done

Domain tests cover cover classification and the directional check. The cursor indicator works. The modifiers stack with Accuracy.
