// IBM Engine: squads, the functions the engine's hooks call.
// A squad is an invisible leader (a copy of the GLA Angry Mob's leader) with an IbmSquadUpdate
// module, plus the members its SpawnBehavior spawned. Every function here does nothing for objects
// that are not squads, so games without the Squads pack play exactly as before.

#pragma once

class AIGroup;
class Squad;
class AsciiString;
class CommandButton;
class ContainModuleInterface;
class GameMessage;
class Image;
class INI;
class Object;
enum CanEnterType CPP_11(: Int);
enum CommandAvailability CPP_11(: Int);
enum CommandSourceType CPP_11(: Int);

namespace IbmEngine
{

// GameLogic::logicMessageDispatcher: a skill or weapon order given to a squad goes to its nearest
// member that has the skill (Burton's C4, Black Lotus's hacks, Kell's shot, the bombers' Detonate).
// Rewrites the message's selection group in place.
void routeSquadCommand(const GameMessage *msg, AIGroup *group);

// ControlBar::getCommandAvailability: a squad's skill buttons are evaluated for the member that has
// the skill, so obj may be replaced. Returns TRUE when the squad decides the button's state itself:
// a skill whose member is dead or inside something.
Bool evaluateSquadCommand(const CommandButton *command, Object *&obj, CommandAvailability &availability);

// AIUpdateInterface::privateAttackObject and privateForceAttackObject: an attack on a squad's trooper
// attacks the whole squad. Fills squad with its troopers and returns TRUE; FALSE for anything else,
// and for skill shots, which keep their one target.
Bool makeSquadTarget(const Object *attacker, const Object *victim, Squad &squad);

// Cheap test for the hook above: TRUE for a squad's leader or trooper.
Bool isSquadMemberOrLeader(const Object *obj);

// AIAttackSquadState::chooseVictim: for a squad made by makeSquadTarget, picks the next trooper to
// shoot (nullptr when none is left), spreading the attackers over the troopers. Returns FALSE for
// other squads (team attacks from scripts), which keep the engine's choice.
Bool chooseSquadVictim(const Object *attacker, Squad *squad, CommandSourceType cmdSource, Bool forceAttack, Object *&victim);

// MobMemberSlavedUpdate::update: TRUE while a member follows its own order (a skill, entering a
// container) or rides inside one, so it must not be pulled back to its leader.
Bool isSquadMemberOnOwnOrder(Object *member, const Object *leader);

// ActionManager::canEnterObject, for mob leaders and members, which may not enter anything in retail.
enum SquadEnterCheck
{
	SQUAD_ENTER_NOT_A_SQUAD,	///< not a squad: keep the retail rule
	SQUAD_ENTER_CHECK_AS_USUAL,	///< a squad member: check it like any infantry
	SQUAD_ENTER_YES,			///< a squad leader whose members fit
	SQUAD_ENTER_NO,				///< a squad leader whose members don't fit
};
SquadEnterCheck checkSquadEnter(const Object *obj, const Object *objectToEnter, CommandSourceType cmdSource, CanEnterType mode);

// AIUpdateInterface::privateEnter: a leader told to enter sends its members in and walks along.
// Returns TRUE when the order was handled here.
Bool onSquadEnter(Object *obj, Object *container, CommandSourceType cmdSource);

// AIUpdateInterface::privateExit: when one member leaves a container, the rest of its squad does too.
void onSquadMemberExit(Object *member, Object *container, CommandSourceType cmdSource);

// TransportContain::isValidContainerFor: the slots a rider needs in a transport. A transport has one
// place per squad it carries; a squad takes one place whatever its size (its first member to board
// needs the place's slots, the rest of the squad nothing). Single soldiers keep their own slots.
Int getTransportRiderSlots(const Object *transport, const ContainModuleInterface *contain, const Object *rider, Int slots);

// TransportContain::onContaining and onRemoving: recounts extraSlots, the slots in use beyond one per
// rider, by places. boarding is a rider being added that is not in the list yet, or nullptr.
// Returns FALSE, leaving extraSlots alone, when Squads is off or the container counts soldiers.
Bool recountTransportSlots(const Object *transport, const ContainModuleInterface *contain, const Object *boarding, Int &extraSlots);

// Drawable::drawContained (the pips) and ControlBar::doTransportInventoryUI: the passenger buttons of
// a transport, one per squad or single rider inside plus one per free place; the riders inside, and
// how many of them are infantry. FALSE when Squads is off or the container counts soldiers.
Bool getSquadTransportButtons(const Object *transport, Int &buttons, Int &riders, Int &infantryRiders);

// ControlBar::populateInvDataCallback: FALSE for a squad member whose squad already has a button in
// the passenger panel. A squad's button shows the squad's picture (image).
Bool showSquadInventoryButton(const Object *transport, const Object *rider, const Image *&image);

// INI block "IbmSquadTransport" (Squads pack): how many squads each transport carries.
void parseSquadTransportDefinition(INI *ini);

// Drawable::drawHealing: a squad shows the heal icon over its health bar while a member is healed
// (the members' own icons are hidden) and just after a lost member came back at the Barracks.
Bool isSquadHealing(const Object *obj);

// TeamTemplateInfo: a team that asks for a basic infantry unit gets the squad that replaces it
// (IbmSquadUpdate's AIReplaces), so the skirmish AI builds squads. The counts are divided by the
// squad's size, rounded up, so the team has about as many soldiers as before.
void replaceTeamUnitWithSquad(AsciiString &unitThingName, Int &minUnits, Int &maxUnits);

// GameLogic::startNewGame, once the players' starting bases are placed: the SquadsDemo test setup.
void setUpSquadsDemo();

} // namespace IbmEngine
