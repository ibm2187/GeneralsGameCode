# 19 Unit scale

**Branch** `coh/unit-scale` · **Needs** 00 · **Slice** `CoH/Features/UnitScale/` · **Flag** `UnitScale`

## Goal

The user, 2026-10-18: "vehicles size are too small compared to infantry". Make vehicles look properly big next to soldiers, in CoH proportions (a tank several soldiers long), without breaking gameplay.

## Behavior

- Vehicles are drawn bigger and infantry a little smaller, by class: tanks, artillery, light vehicles, helicopters, jets, infantry. The factors are at the top of the pack's generator, with presets to compare.
- The logic follows the model: a vehicle's geometry (radii and height) grows with it, so shots hit the hull, units don't overlap and the selection box fits. Infantry keep their radius (spacing, footprint, target size) and shrink in height only.
- A unit's hulk and the add-ons that ride on it take its factor.
- All in an INI pack generated from the retail INI (`Data/make_pack.py`), so the flag off is retail.
- Both ways were weighed: scaling infantry down alone keeps vehicles' footprints (pathfinding, placement) but makes soldiers tiny at Zero Hour's camera height; scaling vehicles up alone needs a large factor. A mix reaches CoH's ratio with smaller changes to each.

## Hook points

None in original code. The engine's own per-object `Scale` (ThingTemplate) and the Geometry fields. A test option, `-cohUnitScaleShowcase`, lines units up for screenshots (hub `gameStartAfterMap`).

## Interactions

Bigger geometry changes pathfinding footprints (most tanks 3 → 4 cells), weapon reach (ranges are measured between bounding circles), Accuracy's target size, and what Cover (05) and Directional armor (07) see. The slice README lists them.

## Done

Replays play clean with the flag off and on (a fresh flag-on replay). The user has watched a windowed skirmish with their flags plus UnitScale and likes the proportions.
