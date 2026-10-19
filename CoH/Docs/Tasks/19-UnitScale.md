# 19 Unit scale

**Branch** `coh/unit-scale` · **Needs** 00 · **Slice** `CoH/Features/UnitScale/` · **Flag** `UnitScale`

## Goal

The user, 2026-10-18: "vehicles size are too small compared to infantry". Make vehicles look properly big next to soldiers, in CoH proportions (a tank several soldiers long), without breaking gameplay.

## Behavior

- Vehicles are drawn bigger next to soldiers, by class: tanks, artillery, heavy machines (dozers, supply trucks), light vehicles, helicopters, jets, infantry. The factors are at the top of the pack's generator, with presets to compare.
- Sizes the user chose in game (2026-10-19), after trying soldiers shrunk to x0.75 to x0.9 ("troops are too small, tanks are too big", "the humvees look huge"): soldiers x1.0, tanks x1.45, artillery x1.3, dozers and supply trucks x1.35, light vehicles x1.05, helicopters x1.25, jets x1.0.
- The logic follows the model: a vehicle's geometry (radii and height) grows with it, so shots hit the hull, units don't overlap and the selection box fits. Infantry keep their radius (spacing, footprint, target size) and shrink in height only.
- A unit's hulk and the add-ons that ride on it take its factor.
- All in an INI pack generated from the retail INI (`Data/make_pack.py`), so the flag off is retail.
- Both ways were weighed: scaling infantry down keeps vehicles' footprints (pathfinding, placement) but makes soldiers tiny at Zero Hour's camera height, which the user saw and rejected; vehicles grow instead, and most tanks need a 5-cell gap like the Overlord.

## Hook points

None in original code. The engine's own per-object `Scale` (ThingTemplate) and the Geometry fields. A test option, `-cohUnitScaleShowcase`, lines units up for screenshots (hub `gameStartAfterMap`).

## Interactions

Bigger geometry changes pathfinding footprints (most tanks 3 → 4 cells), weapon reach (ranges are measured between bounding circles), Accuracy's target size, and what Cover (05) and Directional armor (07) see. The slice README lists them.

## Done

Replays play clean with the flag off and on (a fresh flag-on replay). The user has watched a windowed skirmish with their flags plus UnitScale and likes the proportions.
