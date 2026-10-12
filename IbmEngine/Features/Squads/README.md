# Squads

Infantry fight in squads, like in Company of Heroes. Feature flag: `Squads` (plus `SquadsDemo`, a ready-made test setup). Design notes, including side effects: [Squads and Hard Counters](https://claude.ai/code/artifact/e8ffd0c8-9273-44a0-a357-b3ef7b126972).

The slice has an INI pack (`Data/INI/Features/Squads/`) and C++ for what INI can't do. The C++ does nothing for objects that aren't squads, so games without the pack play exactly as before.

## Rosters

Every squad has a rifle core that fights infantry, plus a specialist. Cost is the members' prices minus 10%.

| Faction | Anti-infantry | Anti-tank | Anti-aircraft | Commando (one at a time, 2,000) |
|---|---|---|---|---|
| USA | Ranger Squad: 4 Rangers (810) | Missile Squad: 2 Rangers + 1 Missile Defender (675) | Stinger Squad: 2 Rangers + 1 AA Defender (675) | Navy SEAL Team: demolitions expert (Burton), 2 riflemen, missile specialist |
| China | Red Guard Squad: 6 Red Guards (810) | Tank Hunter Squad: 3 Red Guards + 2 Tank Hunters (945) | Sky Guard Squad: 3 Red Guards + 2 AA Hunters (945) | Infiltrators: lead (Black Lotus), 2 gunners, missile gunner |
| GLA | Rebel Squad: 5 Rebels (675) | RPG Squad: 2 Rebels + 2 RPG Troopers (810) | Stinger Squad: 2 Rebels + 2 Stinger Troopers (810) | Suicide Squad: marksman (Jarmen Kell), 2 bombers, 2 RPG fighters |

- Squads replace slots 1 and 2 of each barracks (the basic infantry). The anti-aircraft squad and the commandos take empty slots.
- Commandos need the hero's building: Strategy Center, Propaganda Center or Palace. Their members are stealthed while they hold still, and so is the squad's health bar. The commando's buttons are the hero's: Burton's knife and C4, Black Lotus's hacks, Kell's sniper attack, and the bombers' Detonate, which blows all the squad's bombers up on the spot.
- Anti-air specialists fire new air-only missiles: 55 damage, range 225 (the regular infantry missiles do 40 at range 175).
- **The Barracks is the hospital.** It heals friendly infantry within 150, and a squad within 200 of its faction's Barracks gets one lost member back every 2 seconds, for free, specialists first.
- **Attacking a squad attacks the whole squad.** An attack or force attack on any trooper spreads the attackers over the squad's troopers, and they move on to the next trooper until the squad is gone. Skill shots (Kell's snipe, Burton's knife) still hit the man you click.
- **Transports carry squads, not soldiers.** A transport with N slots has N / 4 places (at least one): a Humvee 1, a Troop Crawler or Battle Bus 2, a Chinook 3. A squad takes one place whatever its size; single soldiers and vehicles keep their own slots (a Troop Crawler still comes with 8 Red Guards). The passenger panel shows one button per squad (with the squad's picture) or single rider, plus one per free place, and so do the pips over the transport. The members go in one by one and the invisible leader rides along; when one member gets out (or its button is clicked), the rest follow.
- **Garrisons and tunnels** count soldiers: all members must fit.
- **The skirmish AI builds squads**: AI teams that ask for basic infantry get the squad instead, with team sizes divided by squad size.

## How it works

Each squad is a copy of the GLA Angry Mob's invisible leader (`GLAInfantryAngryMobNexus`), made with `ObjectCopy`, with an `IbmSquadUpdate` module. The leader spawns its members once (`SpawnBehavior`, `OneShot = Yes`), shows one shared health bar (`AggregateHealth`), and picks targets with a harmless weapon. Its members attack what it targets. Selecting any member selects the squad (`KindOf = +IGNORED_IN_GUI` on members). The squad dies when its last member dies.

Members are `ObjectCopy` copies of the retail soldiers with `MobMemberSlavedUpdate` added (`MustCatchUpRadius = 40`, the retail Angry Mob's). `AutoFindHealingUpdate` is removed so injured members don't walk away from their squad. Members that get stealth replace the engine's default infantry stealth module (`ModuleTag_DefaultStealthUpdate`) instead of adding a second one.

| C++ file | What it does |
|---|---|
| `SquadUpdate.cpp/.h` | `IbmSquadUpdate`: members on their own order, parking the leader in containers, leader stealth, reinforcement timer |
| `SquadReinforce.cpp` | Brings back lost members near the Barracks (`ReinforceAt`) |
| `SquadAttack.cpp` | An attack on a trooper becomes an attack on the squad (`AI_ATTACK_SQUAD`, `AI_FORCE_ATTACK_SQUAD`) |
| `SquadCommands.cpp` | Skill orders go to the member with the skill; skill buttons are evaluated for that member |
| `SquadTransport.cpp` | Entering and leaving transports and buildings; transport places (`IbmSquadTransport`), the passenger panel and pips |
| `SquadAI.cpp` | AI teams build squads instead of basic infantry |
| `SquadDemo.cpp` | The `SquadsDemo` test setup |
| `Squads.h` | What the engine hooks call |

| INI file | Contents |
|---|---|
| `20_Weapon.ini` | Leader targeting weapons (ground-only for rifle squads, ground and air for the rest); air-only missiles |
| `30_USA.ini`, `40_China.ini`, `50_GLA.ini` | Members and squads |
| `60_Barracks.ini` | The healing on each faction's Barracks |
| `70_Transport.ini` | How many squads each transport carries (`IbmSquadTransport`: `SlotsPerSquad`, `Transport = <name> <squads>`) |
| `80_CommandButton.ini`, `90_CommandSet.ini` | Build buttons, Detonate; barracks and squad button sets (overrides change only the listed slots) |
| `Squads.str` | Names and tooltips |

## SquadsDemo

With `SquadsDemo` and `Squads` on, every human player of a skirmish or LAN game starts with, toward the map's middle from their Command Center: all three Barracks, one of each squad, a Humvee, Troop Crawler, Technical and Chinook, and $100,000. The first enemy player gets a Rebel, Red Guard and Ranger squad, a Battlemaster, a Technical, a Comanche and a Supply Center a little further on. It has no INI pack, it is placed inside the game logic (so replays work), and loading a saved game doesn't place it again.

## Known limits

- Only USA, China and GLA. The generals' sub-factions (`AirF_`, `Lazr_`, `SupW_`, `Tank_`, `Infa_`, `Nuke_`, `Demo_`, `Chem_`, `Slth_`) keep retail infantry.
- A map with its own `map.str` replaces the pack's tooltips (names stay).
- Lost members come back only at the faction's own Barracks, not the generals' versions of it.

## To do

- **Healing feedback.** A squad healing and getting members back at its Barracks shows it, with an icon over the squad or similar.
- **Members drift apart** (seen after members came back at the Barracks). `MustCatchUpRadius` was 90; now 40. Retest.

## Testing

Checked in play (11 and 12 October 2026): squads build, move and fight; the Barracks heals and refills squads; attacks and force attacks spread over a squad. Headless replays of a squads game (with `SquadsDemo`) and of a game with no features play back with no mismatch. The full checklist is in the test plan doc.

- `generalszh_dbg.exe -headless -file "Maps\AllBuildingsAllSidesUnitTest_Save\AllBuildingsAllSidesUnitTest_Save.map" -feature Squads=1` loads the pack strictly; search `DebugLogFileD.txt` for `IbmEngine` and `ASSERTION`.
- Games started with **Restart Game** don't replay (mismatch on frame 0) with or without squads; that's an existing engine issue.
