# 07 Directional armor

**Branch** `coh/directional-armor` · **Needs** 00 · **Slice** `CoH/Features/DirectionalArmor/` · **Flag** `DirectionalArmor`

## Goal

Flanking tanks pays. Facing the enemy with your strongest armor matters. Tanks are deadly to infantry in the open.

## The user's notes (2026-10-18)

- "Rocket launchers do a lot of damage from the front." Today any rocket wrecks a tank from any angle; frontal armor must stop most of that.
- "We also need tanks to do a lot more damage against infantry."
- "Tanks should shoot like CoH; obliterating infantry."
- After watching the first version: "rear hits and side hits should do more damage; frontal should do less damage", and "the goal is to be close to realistic".

## Behavior

- Vehicles have front, side and rear armor values. Weapons have a penetration value at near and far range.
  Both are set in INI per object and per weapon, with defaults by class (vehicle kind, weapon damage type).
- On a hit, the angle between the shot's origin and the vehicle's facing picks the armor side.
- The chance to penetrate compares penetration and armor, using synced randomness.
- A shot that doesn't penetrate is deflected and deals little or no damage, with a spark effect and an audio cue.
- Rockets mostly deflect off a tank's front, and mostly get through its sides and rear.
- Infantry and buildings are unaffected.
- **Tanks against infantry:** tank main guns and vehicle machine guns do much more damage to infantry, so tanks are deadly to infantry in the open.
  - Done through the pack where possible (Weapon.ini overrides, a damage-type multiplier against infantry) rather than code. The weapons are found in `build/retail/`.
  - The numbers are in INI, with a README table of before and after.
- The skirmish AI attacks vehicles from the side and rear when it can.

## Damage order

hit (Accuracy, at the fire) → cover (Cover, `weaponDamage`) → penetration and the infantry multiplier (this slice, `weaponDamage`) → the engine's armor → veterancy (`damageReceived`).
CONTEXT.md §4 *Combat* keeps the order.

## Likely hook points

`WeaponTemplate::dealDamageInternal`, before each victim's `attemptDamage` (the hub's `weaponDamage`, shared with Cover). Prior art: the old Armor slice on `ibm-engine` did its roll there (CONTEXT §7).

## Done

- Domain tests cover the angle-to-side mapping, the penetration odds and deflection.
- `install.ps1` works; `replaycheck.ps1` passes with the flag off and on.
- The user watched a windowed skirmish at normal speed with their flags plus DirectionalArmor: frontal rockets deflect, flanking works, tanks shred infantry.
- The README, the hooks table and CONTEXT.md are updated.
