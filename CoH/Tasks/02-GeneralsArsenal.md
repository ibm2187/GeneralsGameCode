# 02 General's air and super weapons

**Branch** `coh/generals-arsenal` · **Needs** 00 · **Slice** `CoH/Features/GeneralsArsenal/` · **Flag** `GeneralsArsenal`

## Goal

The sky belongs to the General. Fixed-wing aircraft and super weapons are no longer built.
They are General's powers, called in from off the map and paid for per use with command points earned in combat, as in CoH.
Helicopters stay part of the army.

## Decisions (2026-10-17)

- Air powers are **sorties plus air cover**: one-shot strike runs, and timed patrols over an area.
- Super weapons need **no building**. The counter is a warning that every player sees before impact.
- Powers are **paid per use** with command points. Nothing is unlocked for good by buying it.
- Each plane shot down **lengthens the cooldown** of the power that sent it.

## Behavior

### Command points

- A new per-player currency, earned all the time from combat: the experience your units earn from damage and kills. It is not tied to rank-ups.
  The rates are set in INI.
- Shown next to money, with a tooltip saying how it was earned.
- **Every** General's power is paid this way while the flag is on, including the existing ones (A-10, paradrop, carpet bomb, and so on), so all powers work one way.
  Each power has a point cost, a cooldown and a minimum rank, all set in INI.
- Rank still comes from experience. It gates which powers can be used, in tiers at ranks 1, 3 and 5.
  The power screen shows each power as locked by rank, ready, too expensive or cooling down.
  Powers no longer need to be bought on the science screen.
- Passive promotions (unit unlocks and upgrades such as Paladin or Advanced Training) are still bought with promotion points as now. Only powers move to command points.

### Air sorties

- Jets and bombers can't be built: Raptor, King Raptor, Stealth Fighter, Aurora, MiG and each general's variants.
- Each becomes a sortie power. The planes fly in from the map edge, attack the target area or unit, and leave, with the look and weapons of the unit they replace.

### Air cover

- A few powers are patrols: jets circle an area for 30–60 seconds and attack what enters.
  - An air superiority patrol attacks aircraft, including enemy sorties.
  - A ground attack patrol attacks vehicles.
- The duration, radius and targets are set in INI.

### Losses

- Anti-air shoots down called-in planes as it does now.
- Each plane lost adds time to the cooldown of the power that sent it (set in INI), so air defense and escorts matter.

### Super weapons

- The Particle Cannon, Nuclear Missile and Scud Storm buildings can't be built.
- Each becomes a rank-5 power with a high point cost and a long cooldown, keeping its visuals.
- **Warning:** after targeting there is a delay before impact (set in INI). During it, every player sees the target marked on the ground and the minimap, with a countdown and the voice warning, so units can scatter. This replaces the counterplay of destroying the silo.

### Generals keep their identity

- **Granger (Air Force General):** the cheapest and strongest air powers, plus an extra air cover power.
- **Alexander (Superweapon General):** the cheapest super weapon with the shortest cooldown.
- **Tao (Nuke General):** a stronger nuke.
- **Others:** each general's existing power changes (cheaper or stronger) carry over as cost and strength in INI.
- The README has a table of every power, which generals have it, and its cost, cooldown and rank.

### GLA

- GLA gets no air.
- Scud Storm becomes its super weapon power.
- In place of air it gets ground powers, such as a Scud barrage and a rebel ambush, so it stays asymmetric.

### Helicopters

- Comanche, Helix and Chinook (troop transport) are built at the Airfield or the faction's equivalent.
- The Chinook moves there because Sectors (task 01) removed the Supply Center that built it.
- The Airfield builds only helicopters, and while it stands it is required for that player's air powers, so the airfield is still a target worth hitting.
- GLA gets no helicopters.

### AI

- The skirmish AI earns command points and spends them on powers: strikes on enemy groups and bases, air cover over its army, and super weapons on bases.
- AI build lists drop jets and super weapon buildings.

## Notes from 01

- The Kernel's command set fix lets packs that change the same command set combine, button by button.
- `N = None` empties slot N.
- Sectors already took the supply buildings out of every Dozer and Worker, and pointed prerequisites at the Barracks.

## Likely hook points

- Command sets, power screens and objects come from the pack.
- New special power types go at the end of the enum, so saved IDs don't change.
- Special power use, to check and spend command points.
- Experience gained, to earn command points.
- A plane killed, to add to the cooldown.
- The power screen UI.
- AI scripts and team templates.

New points go in the Kernel hub as general events, recorded in the hooks table.

## Done

- Domain tests cover earning and spending command points, the rank gates, cooldowns with loss penalties, and the super weapon warning timer.
- Every faction and general plays a full skirmish against the AI: powers work, there are no buildable jets or super weapons, helicopters are built at the Airfield, and the AI uses its powers.
- With the flag off, play is unchanged (`CoH/scripts/replaycheck.ps1`).
- The README (with the power table) and the hooks table are updated.
