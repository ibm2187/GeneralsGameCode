// IBM Engine: armor. See Armor.h and README.md.
//
// A vehicle's armor comes from its own "Unit" line (which also covers the generals' versions of it:
// "Tank_ChinaTankOverlord" uses the ChinaTankOverlord line), else the first "Kind" line whose KindOf
// it has, unless it has an Exempt kind. A weapon's penetration comes from its own "Weapon" line, else
// the "Penetration" line for its DamageType; weapons with neither never roll.
//
// The side that is hit comes from where the hitter is, seen from the vehicle: inside the front arc
// is the front, inside the rear arc is the rear, the rest is the side. Rolls use the game logic's
// random numbers, so every PC and every replay sees the same hits and deflections.

#include "PreRTS.h"	// This must go first in EVERY cpp file in the GameEngine

#include "Common/GameCommon.h"
#include "Common/INI.h"
#include "Common/KindOf.h"
#include "Common/NameKeyGenerator.h"
#include "Common/Player.h"
#include "Common/PlayerList.h"
#include "Common/RandomValue.h"
#include "Common/ThingTemplate.h"
#include "GameClient/FXList.h"
#include "GameClient/InGameUI.h"
#include "GameClient/Color.h"
#include "GameLogic/Damage.h"
#include "GameLogic/Object.h"
#include "GameLogic/PartitionManager.h"
#include "GameLogic/Weapon.h"

#include "Features/Armor/Armor.h"
#include "Features/FeatureFlags/FeatureFlags.h"

namespace
{

enum ArmorSide
{
	ArmorSide_Front,
	ArmorSide_Side,
	ArmorSide_Rear,

	ArmorSide_Count
};

struct ArmorValues
{
	Real armor[ArmorSide_Count];
};

struct KindArmor
{
	KindOfType kind;
	ArmorValues values;
};

//-------------------------------------------------------------------------------------------------
// The IbmArmor block's single values. Defaults match the pack.
struct ArmorSettings
{
	Real deflectedDamage;		///< share of a deflected shot's damage that still gets through
	Real frontArc;				///< full width of the front arc, in degrees
	Real rearArc;				///< full width of the rear arc, in degrees
	Real directMaxRadius;		///< weapons with a bigger damage radius are area weapons and never roll
	Real deflectTextPenetration;	///< "Deflected" floats over the vehicle for weapons with at least this penetration (0: never)
	AsciiString deflectFX;		///< FXList played where a deflected shot hits

	ArmorSettings()
	{
		deflectedDamage = 0.10f;
		frontArc = 90.0f;
		rearArc = 90.0f;
		directMaxRadius = 10.0f;
		deflectTextPenetration = 50.0f;
		deflectFX = "FX_IbmArmorDeflect";
	}
};

ArmorSettings s_settings;
Real s_damageTypePenetration[DAMAGE_NUM_TYPES];	///< negative: that DamageType doesn't roll
std::map<NameKeyType, Real> s_weaponPenetration;	///< "Weapon = Name 120" lines
std::vector<KindArmor> s_kinds;						///< "Kind = VEHICLE 50 40 30" lines, in order
KindOfMaskType s_exempt;							///< "Exempt = AIRCRAFT": no Kind armor
std::map<NameKeyType, ArmorValues> s_units;			///< "Unit = AmericaTankCrusader 120 90 60" lines

//-------------------------------------------------------------------------------------------------
ArmorValues parseArmorValues(INI *ini)
{
	ArmorValues values;
	for (Int side = 0; side < ArmorSide_Count; ++side)
		values.armor[side] = INI::scanReal(ini->getNextToken());
	return values;
}

KindOfType parseKind(const char *token)
{
	const Int kind = KindOfMaskType::getSingleBitFromName(token);
	if (kind < 0)
	{
		DEBUG_CRASH(("IbmArmor: unknown KindOf '%s'", token));
		throw INI_INVALID_DATA;
	}
	return (KindOfType)kind;
}

// "Penetration = SMALL_ARMS 5"
void parsePenetrationLine(INI *ini, void * /*instance*/, void * /*store*/, const void * /*userData*/)
{
	const char *name = ini->getNextToken();
	const Int type = DamageTypeFlags::getSingleBitFromName(name);
	if (type < 0)
	{
		DEBUG_CRASH(("IbmArmor: unknown DamageType '%s'", name));
		throw INI_INVALID_DATA;
	}
	s_damageTypePenetration[type] = INI::scanReal(ini->getNextToken());
}

// "Weapon = RangerAdvancedCombatRifle 10"
void parseWeaponLine(INI *ini, void * /*instance*/, void * /*store*/, const void * /*userData*/)
{
	const char *name = ini->getNextToken();
	s_weaponPenetration[NAMEKEY(name)] = INI::scanReal(ini->getNextToken());
}

// "Kind = VEHICLE 50 40 30" (front, side, rear)
void parseKindLine(INI *ini, void * /*instance*/, void * /*store*/, const void * /*userData*/)
{
	const KindOfType kind = parseKind(ini->getNextToken());
	KindArmor line = { kind, parseArmorValues(ini) };
	s_kinds.push_back(line);
}

// "Exempt = AIRCRAFT"
void parseExemptLine(INI *ini, void * /*instance*/, void * /*store*/, const void * /*userData*/)
{
	for (const char *token = ini->getNextTokenOrNull(); token != nullptr; token = ini->getNextTokenOrNull())
		s_exempt.set(parseKind(token));
}

// "Unit = AmericaTankCrusader 120 90 60" (front, side, rear)
void parseUnitLine(INI *ini, void * /*instance*/, void * /*store*/, const void * /*userData*/)
{
	const char *name = ini->getNextToken();
	s_units[NAMEKEY(name)] = parseArmorValues(ini);
}

const FieldParse s_fieldParse[] =
{
	{ "DeflectedDamage",		INI::parsePercentToReal,	nullptr, offsetof(ArmorSettings, deflectedDamage) },
	{ "FrontArc",				INI::parseReal,				nullptr, offsetof(ArmorSettings, frontArc) },
	{ "RearArc",				INI::parseReal,				nullptr, offsetof(ArmorSettings, rearArc) },
	{ "DirectMaxRadius",		INI::parseReal,				nullptr, offsetof(ArmorSettings, directMaxRadius) },
	{ "DeflectTextPenetration",	INI::parseReal,				nullptr, offsetof(ArmorSettings, deflectTextPenetration) },
	{ "DeflectFX",				INI::parseAsciiString,		nullptr, offsetof(ArmorSettings, deflectFX) },
	{ "Penetration",			parsePenetrationLine,		nullptr, 0 },
	{ "Weapon",					parseWeaponLine,			nullptr, 0 },
	{ "Kind",					parseKindLine,				nullptr, 0 },
	{ "Exempt",					parseExemptLine,			nullptr, 0 },
	{ "Unit",					parseUnitLine,				nullptr, 0 },
	{ nullptr, nullptr, nullptr, 0 }
};

//-------------------------------------------------------------------------------------------------
// The vehicle's armor, or nullptr if it has none.
const ArmorValues *findArmor(const ThingTemplate *tmpl)
{
	// Its own line, or the line of the unit it is a general's version of ("Tank_" + name).
	const char *name = tmpl->getName().str();
	while (name != nullptr && *name != '\0')
	{
		std::map<NameKeyType, ArmorValues>::const_iterator it = s_units.find(NAMEKEY(name));
		if (it != s_units.end())
			return &it->second;

		name = strchr(name, '_');
		if (name != nullptr)
			++name;
	}

	if (tmpl->isAnyKindOf(s_exempt))
		return nullptr;

	for (size_t i = 0; i < s_kinds.size(); ++i)
		if (tmpl->isKindOf(s_kinds[i].kind))
			return &s_kinds[i].values;

	return nullptr;
}

//-------------------------------------------------------------------------------------------------
// The weapon's penetration, or a negative number if it never rolls.
Real getPenetration(const WeaponTemplate *weapon)
{
	// Its own line, or the line of the weapon it is a general's version of ("Nuke_" + name).
	const char *name = weapon->getName().str();
	while (name != nullptr && *name != '\0')
	{
		std::map<NameKeyType, Real>::const_iterator it = s_weaponPenetration.find(NAMEKEY(name));
		if (it != s_weaponPenetration.end())
			return it->second;

		name = strchr(name, '_');
		if (name != nullptr)
			++name;
	}

	const DamageType type = weapon->getDamageType();
	if (type < 0 || type >= DAMAGE_NUM_TYPES)
		return -1.0f;
	return s_damageTypePenetration[type];
}

//-------------------------------------------------------------------------------------------------
ArmorSide getSideHit(const Object *victim, const Object *hitter)
{
	const Coord3D *victimPos = victim->getPosition();
	const Coord3D *hitterPos = hitter->getPosition();
	const Real dx = hitterPos->x - victimPos->x;
	const Real dy = hitterPos->y - victimPos->y;
	if (dx * dx + dy * dy < 0.01f)
		return ArmorSide_Side; // right on top: no way to tell

	// Angle between the vehicle's facing and the direction to the hitter, 0 (dead ahead) to 180.
	Real angle = atan2(dy, dx) - victim->getOrientation();
	angle = WWMath::Normalize_Angle(angle);
	const Real degrees = fabs(angle) * 180.0f / PI;

	if (degrees <= s_settings.frontArc / 2.0f)
		return ArmorSide_Front;
	if (degrees >= 180.0f - s_settings.rearArc / 2.0f)
		return ArmorSide_Rear;
	return ArmorSide_Side;
}

//-------------------------------------------------------------------------------------------------
// Sparks where the shot hits, and for anti-tank weapons a "Deflected" note, both only where the
// local player can see (they would give away units under the shroud). Client effects only.
void showDeflection(const Object *victim, const Object *hitter, Real penetration)
{
	const Player *localPlayer = ThePlayerList->getLocalPlayer();
	const Coord3D *victimPos = victim->getPosition();
	if (localPlayer != nullptr && ThePartitionManager->getShroudStatusForPlayer(localPlayer->getPlayerIndex(), victimPos) != CELLSHROUD_CLEAR)
		return;

	// On the vehicle's skin, on the side facing the hitter.
	Coord3D pos = *victimPos;
	Coord3D toHitter = *hitter->getPosition();
	toHitter.sub(*victimPos);
	toHitter.z = 0.0f;
	const Real length = toHitter.length();
	const Real skin = victim->getGeometryInfo().getBoundingCircleRadius() * 0.7f;
	if (length > 0.01f)
	{
		pos.x += toHitter.x / length * skin;
		pos.y += toHitter.y / length * skin;
	}
	pos.z += victim->getGeometryInfo().getMaxHeightAbovePosition() * 0.5f;

	if (!s_settings.deflectFX.isEmpty() && s_settings.deflectFX.compareNoCase("None") != 0)
		FXList::doFXPos(TheFXListStore->findFXList(s_settings.deflectFX.str()), &pos);

	if (TheInGameUI != nullptr && s_settings.deflectTextPenetration > 0.0f && penetration >= s_settings.deflectTextPenetration)
	{
		Coord3D textPos = *victimPos;
		textPos.z += victim->getGeometryInfo().getMaxHeightAbovePosition() + 10.0f;
		TheInGameUI->addFloatingText(UnicodeString(L"Deflected"), &textPos, GameMakeColor(255, 220, 120, 255));
	}
}

} // namespace

//-------------------------------------------------------------------------------------------------
void IbmEngine::parseArmorDefinition(INI *ini)
{
	// Each game's pack sets everything again.
	s_settings = ArmorSettings();
	for (Int i = 0; i < DAMAGE_NUM_TYPES; ++i)
		s_damageTypePenetration[i] = -1.0f;
	s_weaponPenetration.clear();
	s_kinds.clear();
	s_exempt.clear();
	s_units.clear();
	ini->initFromINI(&s_settings, s_fieldParse);
}

//-------------------------------------------------------------------------------------------------
Real IbmEngine::rollArmorPenetration(const WeaponTemplate *weapon, const Object *hitter, const Object *victim, Bool isDirectHit)
{
	if (!isFeatureActive(Feature_Armor) || weapon == nullptr || hitter == nullptr || victim == nullptr || !isDirectHit)
		return 1.0f;

	if (victim->isEffectivelyDead() || victim->isKindOf(KINDOF_STRUCTURE))
		return 1.0f;

	// Blasts (artillery, bombs, super weapons) always get through.
	if (weapon->getPrimaryDamageRadius(WeaponBonus()) > s_settings.directMaxRadius)
		return 1.0f;

	const Real penetration = getPenetration(weapon);
	if (penetration < 0.0f)
		return 1.0f;

	const ArmorValues *armor = findArmor(victim->getTemplate());
	if (armor == nullptr)
		return 1.0f;

	const Real sideArmor = armor->armor[getSideHit(victim, hitter)];
	if (sideArmor <= 0.0f || penetration >= sideArmor)
		return 1.0f;

	if (GameLogicRandomValueReal(0.0f, 1.0f) < penetration / sideArmor)
		return 1.0f;

	showDeflection(victim, hitter, penetration);
	return s_settings.deflectedDamage;
}
