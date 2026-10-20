// CoH hook hub: see Kernel/Hooks.h. Each hook does the Kernel's part of the engine event, then
// raises the event for the subscribed features that are on.

#include "PreRTS.h"	// This must go first in EVERY cpp file in the GameEngine

#include "Common/Money.h"
#include "Common/Player.h"
#include "GameClient/GadgetStaticText.h"
#include "GameClient/GameText.h"
#include "GameLogic/GameLogic.h"
#include "GameLogic/Locomotor.h"
#include "GameLogic/Module/AIUpdate.h"
#include "GameLogic/Object.h"

#include "Kernel/Domain/GameFlags.h"
#include "Kernel/Engine/Internal.h"
#include "Kernel/Hooks.h"
#include "Kernel/Kernel.h"

#include <map>

namespace CoH::Hooks
{

// ---- Events ------------------------------------------------------------------------------------

Event<bool> &gameStartBeforeMap() { static Event<bool> event; return event; }
Event<bool> &gameStartAfterMap() { static Event<bool> event; return event; }
Event<> &logicFrame() { static Event<> event; return event; }
Event<Object *> &objectCreated() { static Event<Object *> event; return event; }
Event<Object *> &objectDestroyed() { static Event<Object *> event; return event; }
Event<> &gameEnd() { static Event<> event; return event; }
Veto<const ThingTemplate *, const Coord3D *, const Player *> &buildLocation()
{
	static Veto<const ThingTemplate *, const Coord3D *, const Player *> veto;
	return veto;
}

Event<Object *, const Object *> &killScored() { static Event<Object *, const Object *> event; return event; }
Event<Object *, const DamageInfo *> &damageDealt() { static Event<Object *, const DamageInfo *> event; return event; }
Veto<const Object *, const SpecialPowerTemplate *> &specialPowerUse()
{
	static Veto<const Object *, const SpecialPowerTemplate *> veto;
	return veto;
}
Event<Object *, const SpecialPowerTemplate *, const Coord3D *> &specialPowerTriggered()
{
	static Event<Object *, const SpecialPowerTemplate *, const Coord3D *> event;
	return event;
}
Veto<const Player *, const ThingTemplate *> &production()
{
	static Veto<const Player *, const ThingTemplate *> veto;
	return veto;
}
Event<const Player *, unsigned int &> &income() { static Event<const Player *, unsigned int &> event; return event; }
Event<Object *, Object *> &unitProduced() { static Event<Object *, Object *> event; return event; }
Event<const Player *, const ThingTemplate *, int &> &buildCost()
{
	static Event<const Player *, const ThingTemplate *, int &> event;
	return event;
}
Event<const Player *, const ThingTemplate *, int &> &buildTime()
{
	static Event<const Player *, const ThingTemplate *, int &> event;
	return event;
}
Event<Player *, Object *, bool> &playerSelection() { static Event<Player *, Object *, bool> event; return event; }
Event<Object *, const AICommandParms *> &aiCommand() { static Event<Object *, const AICommandParms *> event; return event; }
Event<const Object *, const Object *, int &, int &> &containRoom()
{
	static Event<const Object *, const Object *, int &, int &> event;
	return event;
}
Event<WeaponShot &> &weaponFiringAtObject() { static Event<WeaponShot &> event; return event; }
Event<const Object *, const WeaponTemplate *, WeaponBonus &> &weaponBonus()
{
	static Event<const Object *, const WeaponTemplate *, WeaponBonus &> event;
	return event;
}
Event<const Object *, const WeaponTemplate *, const Coord3D &, float> &weaponDetonated()
{
	static Event<const Object *, const WeaponTemplate *, const Coord3D &, float> event;
	return event;
}
Event<WeaponDamage &> &weaponDamage() { static Event<WeaponDamage &> event; return event; }
Event<const Object *, const DamageInfo *, float &> &damageReceived()
{
	static Event<const Object *, const DamageInfo *, float &> event;
	return event;
}
Event<Object *, int, int> &veterancyChanged() { static Event<Object *, int, int> event; return event; }
Event<const Object *, float &> &unitSpeed() { static Event<const Object *, float &> event; return event; }
Event<const Object *, Handling, float &> &unitHandling()
{
	static Event<const Object *, Handling, float &> event;
	return event;
}

Event<> &inGameUIPostDraw() { static Event<> event; return event; }
Event<int, int, int, int> &radarDraw() { static Event<int, int, int, int> event; return event; }
Event<const Player *, UnicodeString &> &moneyTooltip() { static Event<const Player *, UnicodeString &> event; return event; }
Event<const CommandButton *, UnicodeString &> &commandTooltip() { static Event<const CommandButton *, UnicodeString &> event; return event; }
Event<const Player *, UnicodeString &> &moneyDisplay() { static Event<const Player *, UnicodeString &> event; return event; }
Event<const Object *, const IRegion2D *, float &, float &> &healthBar()
{
	static Event<const Object *, const IRegion2D *, float &, float &> event;
	return event;
}
Veto<const Object *, const Object *> &riderShown()
{
	static Veto<const Object *, const Object *> veto;
	return veto;
}

// ---- Engine ------------------------------------------------------------------------------------

// A locomotor doesn't know its object, so the hub remembers whose current locomotor each one is.
// The map is only looked up, never walked, and an entry counts only while that object is alive and
// the locomotor is still its current one, so a freed locomotor's address reused by another object's
// is never taken for the old owner's.
namespace
{
std::map<const Locomotor *, ObjectID> &locomotorOwners()
{
	static std::map<const Locomotor *, ObjectID> owners;
	return owners;
}

bool wantsLocomotorOwners()
{
	return unitSpeed().hasActiveSubscriber() || unitHandling().hasActiveSubscriber();
}

// The living object whose current locomotor this is, or null.
const Object *ownerOf(const Locomotor *locomotor)
{
	if (locomotorOwners().empty())
		return nullptr;
	const auto found = locomotorOwners().find(locomotor);
	if (found == locomotorOwners().end())
		return nullptr;
	const Object *owner = TheGameLogic->findObjectByID(found->second);
	const AIUpdateInterface *ai = owner != nullptr ? owner->getAI() : nullptr;
	if (ai == nullptr || ai->getCurLocomotor() != locomotor)
		return nullptr;
	return owner;
}
}

// Quick test skirmishes (-cohSkirmish) may run side by side, as replay playback may, and so may other
// test runs started with -cohSeveralGames (loading a test's save with -loadsave). Each game then gets
// its own replay (the engine's _InstanceNN names) and its own CoH log.
bool allowsSeveralGames()
{
	return QuickSkirmish::isAsked() || hasCommandLineOption("-cohSeveralGames");
}

void onEngineInit()
{
	Kernel::init();
	QuickSkirmish::prepareIfAsked();
}

void onShellShown()
{
	QuickSkirmish::startIfAsked();
}

void onModuleFactoryInit(ModuleFactory &factory)
{
	Registration::addModulesTo(factory);
}

INIBlockParse findIniBlock(const char *token)
{
	return Registration::findIniBlock(token);
}

// ---- Game --------------------------------------------------------------------------------------

void onGameStartBeforeMap(bool loadingSaveGame)
{
	FeatureRegistry &registry = Kernel::registry();
	registry.activate(registry.known(GameFlags::forNewGame(loadingSaveGame)));
	GameFlags::clearSaved();
	DEBUG_LOG(("CoH: game starts with flags '%s'", registry.active().toList().c_str()));

	Packs::loadActive(registry);
	gameStartBeforeMap().raise(loadingSaveGame);
}

void onGameStartAfterMap(bool loadingSaveGame)
{
	gameStartAfterMap().raise(loadingSaveGame);
}

void onLogicFrame()
{
	logicFrame().raise();
	Stats::logIfDue();
	QuickSkirmish::quitIfDue();
}

void onObjectCreated(Object *object)
{
	objectCreated().raise(object);
}

void onObjectDestroyed(Object *object)
{
	objectDestroyed().raise(object);
}

// The engine resets after every game and once at startup. The packs' overrides are dropped by the
// engine's own reset right after this, like map.ini's.
void onGameEnd()
{
	gameEnd().raise();
	locomotorOwners().clear();

	FeatureRegistry &registry = Kernel::registry();
	registry.resetAll();
	registry.deactivateAll();
	GameFlags::clearSaved();
	PackText::clear();
	Stats::reset();
}

bool isBuildLocationAllowed(const ThingTemplate *build, const Coord3D *position, const Player *player,
	const Object *builder)
{
	if (player == nullptr && builder != nullptr)
		player = builder->getControllingPlayer();
	return buildLocation().allows(build, position, player);
}

void onKillScored(Object *killer, const Object *victim)
{
	killScored().raise(killer, victim);
}

void onDamageDealt(Object *victim, const DamageInfo *damage)
{
	damageDealt().raise(victim, damage);
}

bool isSpecialPowerAllowed(const Object *source, const SpecialPowerTemplate *power)
{
	return specialPowerUse().allows(source, power);
}

void onSpecialPowerTriggered(Object *source, const SpecialPowerTemplate *power, const Coord3D *target)
{
	specialPowerTriggered().raise(source, power, target);
}

bool isProductionAllowed(const Player *player, const ThingTemplate *build)
{
	return production().allows(player, build);
}

void onIncome(const Player *player, unsigned int &amount)
{
	if (player == nullptr)
		return;
	income().raise(player, amount);
	Stats::onIncome(*player, amount);
}

void onUnitProduced(Object *factory, Object *unit)
{
	unitProduced().raise(factory, unit);
}

void onBuildCost(const Player *player, const ThingTemplate *build, int &cost)
{
	buildCost().raise(player, build, cost);
}

void onBuildTime(const Player *player, const ThingTemplate *build, int &frames)
{
	buildTime().raise(player, build, frames);
}

void onPlayerSelection(Player *player, Object *object, bool selected)
{
	if (player != nullptr && object != nullptr)
		playerSelection().raise(player, object, selected);
}

void onAiCommand(Object *object, const AICommandParms *command)
{
	if (object != nullptr)
		aiCommand().raise(object, command);
}

void onContainRoom(const Object *container, const Object *rider, int &used, int &needed)
{
	containRoom().raise(container, rider, used, needed);
}

bool onWeaponFiringAtObject(const Object *source, const WeaponTemplate *weapon, const WeaponBonus &bonus,
	const Object *victim, Coord3D &spot)
{
	if (source == nullptr || victim == nullptr || !weaponFiringAtObject().hasActiveSubscriber())
		return false;
	WeaponShot shot = { source, weapon, &bonus, victim, &spot, false };
	weaponFiringAtObject().raise(shot);
	return shot.retargeted;
}

void onWeaponBonus(const Object *source, const WeaponTemplate *weapon, WeaponBonus &bonus)
{
	weaponBonus().raise(source, weapon, bonus);
}

void onWeaponDetonated(const Object *source, const WeaponTemplate *weapon, const Coord3D *position, float radius)
{
	if (position != nullptr)
		weaponDetonated().raise(source, weapon, *position, radius);
}

void onWeaponDamage(const Object *source, const WeaponTemplate *weapon, const Coord3D *position, Object *victim,
	DamageInfo *damage)
{
	if (victim == nullptr || damage == nullptr || !weaponDamage().hasActiveSubscriber())
		return;
	WeaponDamage hit = { source, weapon, position, victim, damage };
	weaponDamage().raise(hit);
}

void onDamageReceived(const Object *victim, const DamageInfo *damage, float &amount)
{
	if (victim != nullptr && damage != nullptr)
		damageReceived().raise(victim, damage, amount);
}

void onVeterancyChanged(Object *object, int oldLevel, int newLevel)
{
	if (object != nullptr && oldLevel != newLevel)
		veterancyChanged().raise(object, oldLevel, newLevel);
}

void onLocomotorChosen(const Object *object, const Locomotor *locomotor)
{
	if (object == nullptr || locomotor == nullptr || !wantsLocomotorOwners())
		return;
	locomotorOwners()[locomotor] = object->getID();
}

void onLocomotorSpeed(const Locomotor *locomotor, float &speed)
{
	if (!unitSpeed().hasActiveSubscriber())
		return;
	if (const Object *owner = ownerOf(locomotor))
		unitSpeed().raise(owner, speed);
}

void onLocomotorHandling(const Locomotor *locomotor, Handling handling, float &value)
{
	if (!unitHandling().hasActiveSubscriber())
		return;
	if (const Object *owner = ownerOf(locomotor))
		unitHandling().raise(owner, handling, value);
}

// ---- Client ------------------------------------------------------------------------------------

void onInGameUIPostDraw()
{
	inGameUIPostDraw().raise();
}

void onRadarDraw(int x, int y, int width, int height)
{
	radarDraw().raise(x, y, width, height);
}

void onHealthBar(const Object *object, const IRegion2D *bar, float &health, float &maxHealth)
{
	healthBar().raise(object, bar, health, maxHealth);
}

bool isRiderShown(const Object *container, const Object *rider)
{
	return riderShown().allows(container, rider);
}

namespace
{
GradientQuadDraw theGradientQuad = nullptr;
}

void provideGradientQuad(GradientQuadDraw draw)
{
	theGradientQuad = draw;
}

bool drawGradientQuad(const float corners[8], unsigned int color1, unsigned int color2)
{
	if (theGradientQuad == nullptr)
		return false;
	theGradientQuad(corners, color1, color2);
	return true;
}

void onMoneyTooltip(const Player *player, UnicodeString &description)
{
	moneyTooltip().raise(player, description);
}

void onCommandTooltip(const CommandButton *button, UnicodeString &description)
{
	commandTooltip().raise(button, description);
}

// With a feature adding to it, the money text is made here every frame: what features add (such as
// command points) changes without the money changing, which is all the original code watches.
bool onMoneyDisplay(const Player *player, GameWindow *moneyWindow)
{
	if (!moneyDisplay().hasActiveSubscriber() || player == nullptr || moneyWindow == nullptr)
		return false;

	UnicodeString text;
	text.format(TheGameText->fetch("GUI:ControlBarMoneyDisplay"), player->getMoney()->countMoney());
	moneyDisplay().raise(player, text);

	static UnicodeString shown;
	if (text != shown || GadgetStaticTextGetText(moneyWindow) != text)
	{
		GadgetStaticTextSetText(moneyWindow, text);
		shown = text;
	}
	return true;
}

const UnicodeString *findText(const char *label)
{
	return PackText::find(label);
}

// ---- Game options ------------------------------------------------------------------------------

AsciiString gameOptions(const GameInfo *game)
{
	return GameFlags::optionsFor(game);
}

bool isGameOptionKey(const AsciiString &key)
{
	return GameOptionKey == key.str();
}

void onGameOptionsParsed(GameInfo *game, const AsciiString &options)
{
	GameFlags::onOptionsParsed(game, options);
}

} // namespace CoH::Hooks
