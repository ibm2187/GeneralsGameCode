# Squads

Infantry fight in squads, like in Company of Heroes. Feature flag: `Squads`. Design notes, including side effects: [Squads and Hard Counters](https://claude.ai/code/artifact/e8ffd0c8-9273-44a0-a357-b3ef7b126972).

Everything is in the INI pack `Data/INI/Features/Squads/`; this slice has no C++ yet.

## Rosters

Every squad has a rifle core that fights infantry, plus a specialist. Cost is the members' prices minus 10%.

| Faction | Anti-infantry | Anti-tank | Anti-aircraft | Commando (one at a time, 2,000) |
|---|---|---|---|---|
| USA | Ranger Squad: 4 Rangers (810) | Missile Squad: 2 Rangers + 1 Missile Defender (675) | Stinger Squad: 2 Rangers + 1 AA Defender (675) | Navy SEAL Team: demolitions expert (Burton), 2 riflemen, missile specialist |
| China | Red Guard Squad: 6 Red Guards (810) | Tank Hunter Squad: 3 Red Guards + 2 Tank Hunters (945) | Sky Guard Squad: 3 Red Guards + 2 AA Hunters (945) | Infiltrators: lead (Black Lotus), 2 gunners, missile gunner |
| GLA | Rebel Squad: 5 Rebels (675) | RPG Squad: 2 Rebels + 2 RPG Troopers (810) | Stinger Squad: 2 Rebels + 2 Stinger Troopers (810) | Suicide Squad: marksman (Jarmen Kell), 2 bombers, 2 RPG fighters |

- Squads replace slots 1 and 2 of each barracks (the basic infantry). The anti-aircraft squad and the commandos take empty slots.
- Commandos need the hero's building: Strategy Center, Propaganda Center or Palace. Their members are stealthed while they hold still. GLA bombers explode when killed.
- Anti-air specialists fire new air-only missiles: 55 damage, range 225 (the regular infantry missiles do 40 at range 175).
- **Medical Center** (800, needs a Barracks), built by the dozer or worker: heals friendly infantry within 150, and infantry can go inside to heal. It reuses the barracks model.

## How it works

Each squad is a copy of the GLA Angry Mob's invisible leader (`GLAInfantryAngryMobNexus`), made with `ObjectCopy`. The leader spawns its members once (`SpawnBehavior`, `OneShot = Yes`), shows one shared health bar (`AggregateHealth`), and picks targets with a harmless weapon. Its members attack what it targets. Selecting any member selects the squad (`KindOf = +IGNORED_IN_GUI` on members). The squad dies when its last member dies.

Members are `ObjectCopy` copies of the retail soldiers with `MobMemberSlavedUpdate` added. `AutoFindHealingUpdate` is removed so injured members don't walk away from their squad.

| File | Contents |
|---|---|
| `20_Weapon.ini` | Leader targeting weapons (ground-only for rifle squads, ground and air for the rest); air-only missiles |
| `30_USA.ini`, `40_China.ini`, `50_GLA.ini` | Members and squads |
| `60_MedicalCenter.ini` | Medical Centers |
| `80_CommandButton.ini`, `90_CommandSet.ini` | Build buttons; barracks and dozer button sets (overrides change only the listed slots) |
| `Squads.str` | Names and tooltips |

## Known limits

- Commando hero skills (C4, hacks, kill-driver shot) don't work yet: the abilities belong to members, but you command the leader. Needs skill routing in C++.
- Squads can't enter transports or garrison buildings; the leader has `TransportSlotCount = 0`.
- Lost members aren't replaced. Reinforcing at the Medical Center needs C++.
- The skirmish AI still builds single infantry; it builds from its own lists, not the barracks buttons.
- Only USA, China and GLA. The generals' sub-factions (`AirF_`, `Lazr_`, `SupW_`, `Tank_`, `Infa_`, `Nuke_`, `Demo_`, `Chem_`, `Slth_`) keep retail infantry.
- A map with its own `map.str` replaces the pack's tooltips (names stay).

## Testing

Tested on 11 October 2026 in skirmish (China on Alpine Assault):

- Squads build, spawn their members, select as one unit, move and attack-move together.
- The Medical Center builds, shows its name and requires a Barracks.
- Ending a game, restarting and exiting to the menu work; the pack loads again in the next game.
- A replay of a squads game plays back headless with no mismatch (`generalszh.exe -headless -replay 00000000.rep`).
- Games started with **Restart Game** don't replay (mismatch on frame 0) with or without squads; that's an existing engine issue.

Not yet checked in a game: USA and GLA squads, commandos, anti-air squads shooting aircraft, and Medical Center healing.
