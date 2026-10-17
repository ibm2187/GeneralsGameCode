# 08 Vehicle criticals

**Branch** `coh/vehicle-criticals` · **Needs** 07 · **Slice** `CoH/Features/VehicleCriticals/` · **Flag** `VehicleCriticals`

## Goal

Damaged vehicles get worse before they die, and are worth saving and repairing.

## Behavior

- A penetrating hit may roll a critical effect, more likely at low health:
  - **Engine damaged:** slower.
  - **Immobilized:** can't move.
  - **Main gun destroyed:** main weapon offline.
  - **Crew shocked:** briefly can't act.
- Repairs clear them. Icons show each one.
- The AI repairs or withdraws crippled vehicles.

## Done

Domain tests cover the critical tables. Each critical works and is cleared by repair.
