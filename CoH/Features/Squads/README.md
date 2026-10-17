# Squads

**Flag** `Squads` · **Task** [09](../../Docs/Tasks/09-Squads.md) · **Public header** [Squads.h](Squads.h)

Infantry fights as squads, as in Company of Heroes: one order buys several soldiers, who are selected, ordered, carried and counted as one unit.

## How it plays

- **One order, one squad.** Ordering a unit the pack lists (`Squad = <unit> <size>`) makes a squad of that many: the factory makes the ordered soldier, and the rest walk out of the same door to the same rally point. A general's version of a unit (`AirF_AmericaInfantryRanger`) is made the same way unless the pack lists it on its own. A squad costs its members' price times `CostPercent` (90%) and takes their build time times `TimePercent` (60%): four Rangers cost 810 and take 12 seconds. China's Barracks makes Red Guards two per order; one order is still one squad of six.
- **Heroes stay single.** A unit with no line is made alone: Burton, Black Lotus, Jarmen Kell, workers, hijackers, snipers, hackers. A hero may instead get an escort (`Squad = GLAInfantryJarmenKell 1 GLAInfantryRebel 2`), which comes with the hero and costs its own price on top.
- **One selection.** Clicking or boxing any member selects its whole squad, and deselecting one deselects them all. Every order then goes to every member.
- **Formation.** A squad sent somewhere (move, attack-move) spreads over formation slots around the goal: rows of `RowWidth` (3) across the way it is heading, `Spacing` (22) apart, the rows behind shifted half a space. A squad keeps the order of its slots, so the same soldier takes the same place each time. Cover (task 05) will move slots onto cover.
- **Transports and garrisons** count a squad as one unit: it takes the room of its first member (a Ranger squad takes one Humvee slot), and the rest ride along. When a member gets out, the mates that rode with it get out too.
- **Population** (Upkeep) counts a squad once, by the population of its whole price: a Ranger squad (810) takes 3.
- **The health bar** is the squad's: the members' health together, out of the full squad's. A pip under it for each member, dark for the lost ones. The leader shows it; while the leader isn't selected, the member under the cursor does.
- **A lost member** leaves its slot empty; the squad fights on with the rest, and is gone with its last member. Reinforce (task 10) fills empty slots.

## Skirmish AI

The AI orders units through the same factories, so it builds squads without knowing it. The members join the AI team the ordered unit joined, so the team's orders (attack, guard, hunt, garrison) reach every member, and Upkeep's army limit counts squads as it counts the player's.

## Settings (`CoHSquads` in [Data/INI/10_Squads.ini](Data/INI/10_Squads.ini))

| Setting | Default | Meaning |
|---|---|---|
| `Squad = <unit> <size> [<escort> <count>]...` | none | ordering the unit makes a squad of this many, plus any escorts; no line means it is made alone |
| `CostPercent` | 90 | a squad costs its members' price times this |
| `TimePercent` | 60 | a squad takes its members' build time times this |
| `Spacing` | 22 | distance between neighbours in formation |
| `RowWidth` | 3 | members in one formation row |

With no `Squad` line the feature does nothing. The pack's squads are: Rangers 4, Missile Defenders 3, Red Guards 6, Tank Hunters 3, Mini-Gunners 3, Rebels 5, Tunnel Defenders 3.

## For other slices (`Squads.h`)

- Who: `squadOf(object)`, `members(squad)`, `leader(squad)`, `countsAsUnit(object)` (Upkeep).
- What was lost and refilling it (Reinforce, Weapon pickup): `slotCount`, `missingMembers`, `emptySlots`, `unitOfSlot(squad, slot)`, `addMember(squad, object)`.
- Orders (Retreat, Abilities): `moveTo(squad, goal)` sends it in formation; `formationSlot(member, &position)` is where its slot put it (Cover).
- Cost (Upkeep): `isSquadUnit(unit)`, `buildCostOf(unit)`.

## Hooks

| Hub point | Used for |
|---|---|
| `unitProduced` (`ProductionUpdate::update`) | makes the rest of the squad at the factory, in the unit's team, out of the same door |
| `buildCost`, `buildTime` (`ThingTemplate::calcCostToBuild`, `calcTimeToBuild`) | the squad's price and build time, for buttons, queue, refunds and the AI |
| `playerSelection` (`GameLogicDispatch`, select and deselect messages) | selecting a member selects the squad, in game logic and, for the local player, on screen |
| `aiCommand` (`AIUpdateInterface::aiDoCommand`) | collects moves to send in formation at the end of the frame, and exits for the mates to follow |
| `containRoom` (`TransportContain`, `GarrisonContain::isValidContainerFor`) | a squad takes the room of one unit |
| `healthBar` (`Drawable::drawHealthBar`) | the squad's health, the member pips, one bar per squad |
| `logicFrame`, `objectDestroyed`, `gameStartAfterMap`, `gameEnd` | carrying out the frame's orders, losing members, the game's squads |

## Engine notes

- **Selection.** A player's selection lives in game logic (`Player::m_currentSelection`, a `Squad` of object IDs) and orders use it, so expanding it there reaches every machine. `GameLogic::selectObject`/`deselectObject` are also called on death and by scripts, so the hub point is in the message handlers (`GameLogic::onCreateSelectedGroup`, `onRemoveFromSelectedGroup`) instead. `Squad::addObjectID` doesn't check for duplicates: deselect a mate before selecting it. `affectClient` set for the local player's own objects selects their drawables on screen (as `GameLogic.cpp` 2456 does).
- **Group moves.** `AIGroup::groupMoveToPosition` sends infantry with `AICMD_FOLLOW_PATH` (`friend_moveInfantryToPos`), each member its own path, so a squad's goal is the average of its members' path ends. Re-issuing orders inside `aiDoCommand` would re-enter it; the slice collects them and re-issues at the end of the logic frame, with a flag so its own orders aren't collected.
- **Production.** `ProductionUpdate::update` makes the unit, calls `ExitInterface::exitObjectViaDoor`, `Player::onUnitCreated` (the AI's `onUnitProduced` puts it in its team) and the create modules' `onBuildComplete`, then counts it. `exitObjectViaDoor` may be called again for more objects with `DOOR_1`. `QuantityModifier` on the factory's `ProductionUpdate` (Red Guards 2) makes several per order; the entry's `getProductionQuantityRemaining` still counts the one just made when the hook runs.
- **Containers.** Transports count slots (`m_extraSlotsInUse + getContainCount() + rider->getTransportSlotCount() <= max`); garrisons count objects. A garrison's fire points come from its model, so members past them sit inside without firing. Tunnels (`CaveContain`) are not changed: they count soldiers.
- **Cost.** `calcCostToBuild`/`calcTimeToBuild` serve the buttons, tooltips, the queue, cancel refunds and the AI alike. `ThingTemplate::getBuildTime` is protected; escorts' build time is worked out from their price.

## Testing

- Domain tests: `z_googletest.exe --gtest_filter=Squads*` (formation slots, member bookkeeping, squad types and price, container room).
- In-game self-test: play a skirmish as USA with `-cohSquadsSelfTest -cohLog` (and `-coh Sectors,Upkeep,Squads`). Two Ranger squads, a Humvee and a pub appear near the command center, and the test sends the messages the player's clicks send: it checks the squad's price and population, that a click selects the squad and a box over two squads selects each member once, that deselecting one deselects the squad, the formation at the goal, riding in the Humvee and getting out together, garrisoning and evacuating the pub, and a lost member's empty slot. Grep `CoH.log` (or `CoH_InstanceNN.log`) for `Squads test`; the last line is the summary. Its replay doesn't play back (the test makes its objects directly).
- With `-cohLog` the slice logs each squad made, a later unit of an order joining its squad (Red Guards), and each squad sent into a transport or building.

## Known limits

- Squads are not saved: a loaded save has single soldiers.
- The passenger panel and the pips over a transport show one per soldier.
- An attack on a squad member is an attack on that soldier; attackers don't spread over the squad.
- Infantry that comes another way (a Troop Crawler's Red Guards, paradrops, scripts) is single.
