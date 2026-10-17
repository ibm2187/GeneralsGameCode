# 02 General's air and super weapons

**Branch** `coh/generals-arsenal` · **Needs** 00 · **Slice** `CoH/Features/GeneralsArsenal/` · **Flag** `GeneralsArsenal`

## Goal

The sky belongs to the General. Fixed-wing aircraft and super weapons are no longer built.
They are General's powers, bought with General's points and called in from off the map. Helicopters stay part of the army.

## Behavior

- **Jets:** jets and bombers can't be built (Raptor, King Raptor, Stealth Fighter, Aurora, MiG and each general's variants).
  Each becomes a power: a sortie flies in, attacks the target area or unit, and leaves.
  These sit beside the existing strikes (A-10, carpet bomb, and so on).
- **Super weapons:**
  - Particle Cannon, Nuclear Missile and Scud Storm buildings can't be built.
  - Each becomes a top-rank General's power with a long cooldown, keeping its visuals and the warning to all players.
  - The rank and cost are set in INI.
- **General's points:**
  - Points are still earned by fighting.
  - Each faction's General's power screen is extended with the new air and super weapon powers.
  - A faction without air (GLA) gets its own equivalents instead: for example Scud barrages and rebel ambushes.
- **Helicopters:**
  - Comanche, Helix and Chinook (troop transport) are built at the Airfield or the faction's equivalent.
  - The Airfield builds only helicopters and is required for the air powers.
  - GLA gets no helicopters unless decided otherwise.
- **Air defense:** anti-air units and buildings can shoot down incoming sorties as they do now, so air defense still matters.
- **AI:** the skirmish AI buys and uses the new powers. AI build lists drop jets and super weapon buildings.

## Likely hook points

- Command sets and General's power screens come from the pack.
- New power types are added to the special power type list. Append them to the end so saved IDs don't change.
- Special power targeting.
- AI team templates.

## Done

- Every faction and general plays a full skirmish with its powers working and no buildable jets or super weapons.
- The AI uses the powers.
- Domain tests cover the power cost and rank rules.
