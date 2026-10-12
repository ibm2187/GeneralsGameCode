// IBM Engine: economy. See Economy.h and README.md.
//
// A unit's multipliers come from the first "Kind" line whose KindOf it has (only for units a
// player can build, and not for the Exempt kinds), then its own "Unit" line replaces any stat that
// line names. A Unit line also matches the generals' versions of the unit: "Tank_ChinaTankOverlord"
// uses the line for "ChinaTankOverlord" unless it has its own.

#include "PreRTS.h"	// This must go first in EVERY cpp file in the GameEngine

#include "Common/INI.h"
#include "Common/KindOf.h"
#include "Common/NameKeyGenerator.h"
#include "Common/ThingTemplate.h"
#include "GameLogic/Damage.h"
#include "GameLogic/Locomotor.h"
#include "GameLogic/LocomotorSet.h"
#include "GameLogic/Object.h"
#include "GameLogic/Weapon.h"
#include "GameLogic/Module/BodyModule.h"

#include "Features/Economy/Economy.h"
#include "Features/FeatureFlags/FeatureFlags.h"

namespace
{

using IbmEngine::EconomyStat;
using IbmEngine::EconomyStat_Count;

const char *const s_statNames[] = { "Cost", "BuildTime", "Health", "Damage", "Speed", "RateOfFire", "VsInfantry" };
static_assert(ARRAY_SIZE(s_statNames) == EconomyStat_Count, "s_statNames must match EconomyStat");

//-------------------------------------------------------------------------------------------------
// One line's multipliers; a negative value means the line doesn't set that stat.
struct EconomyScales
{
	Real scale[EconomyStat_Count];

	EconomyScales(Real value)
	{
		for (Int i = 0; i < EconomyStat_Count; ++i)
			scale[i] = value;
	}
};

struct KindScales
{
	KindOfType kind;
	EconomyScales scales;
};

std::vector<KindScales> s_kinds;					///< "Kind = VEHICLE Cost:200% ..." lines, in order
KindOfMaskType s_exempt;							///< "Exempt = DOZER HARVESTER": no Kind multipliers
std::map<NameKeyType, EconomyScales> s_units;		///< "Unit = ChinaTankOverlord Speed:70%" lines
std::map<const ThingTemplate *, EconomyScales> s_cache;
std::map<NameKeyType, Real> s_reloadSeconds;		///< "Reload = CrusaderTankGun 6" lines
std::map<const WeaponTemplate *, Int> s_reloadCache;	///< frames, or -1
std::map<Int, Real> s_vsInfantryByDamage;			///< "VsInfantryByDamage = ARMOR_PIERCING 1000%" lines, by DamageType

//-------------------------------------------------------------------------------------------------
// Reads "Stat:Percent" tokens up to the end of the line.
EconomyScales parseScales(INI *ini)
{
	EconomyScales scales(-1.0f);
	for (const char *token = ini->getNextTokenOrNull(); token != nullptr; token = ini->getNextTokenOrNull())
	{
		const char *colon = strchr(token, ':');
		Int stat = 0;
		while (colon != nullptr && stat < EconomyStat_Count
			&& !(strlen(s_statNames[stat]) == (size_t)(colon - token) && strnicmp(token, s_statNames[stat], colon - token) == 0))
			++stat;

		if (colon == nullptr || stat == EconomyStat_Count)
		{
			DEBUG_CRASH(("IbmEconomy: '%s' should be Stat:Percent, with Stat one of Cost, BuildTime, Health, Damage, Speed, RateOfFire, VsInfantry", token));
			throw INI_INVALID_DATA;
		}
		scales.scale[stat] = INI::scanPercentToReal(colon + 1);
	}
	return scales;
}

//-------------------------------------------------------------------------------------------------
KindOfType parseKind(const char *token)
{
	const Int kind = KindOfMaskType::getSingleBitFromName(token);
	if (kind < 0)
	{
		DEBUG_CRASH(("IbmEconomy: unknown KindOf '%s'", token));
		throw INI_INVALID_DATA;
	}
	return (KindOfType)kind;
}

//-------------------------------------------------------------------------------------------------
void parseKindLine(INI *ini, void * /*instance*/, void * /*store*/, const void * /*userData*/)
{
	const KindOfType kind = parseKind(ini->getNextToken());
	KindScales line = { kind, parseScales(ini) };
	s_kinds.push_back(line);
}

//-------------------------------------------------------------------------------------------------
void parseExemptLine(INI *ini, void * /*instance*/, void * /*store*/, const void * /*userData*/)
{
	for (const char *token = ini->getNextTokenOrNull(); token != nullptr; token = ini->getNextTokenOrNull())
		s_exempt.set(parseKind(token));
}

//-------------------------------------------------------------------------------------------------
void parseUnitLine(INI *ini, void * /*instance*/, void * /*store*/, const void * /*userData*/)
{
	const char *name = ini->getNextToken();
	s_units.insert(std::make_pair(NAMEKEY(name), parseScales(ini)));
}

//-------------------------------------------------------------------------------------------------
// "Reload = CrusaderTankGun 6": seconds between shots, or to reload the weapon's clip.
void parseReloadLine(INI *ini, void * /*instance*/, void * /*store*/, const void * /*userData*/)
{
	const char *name = ini->getNextToken();
	s_reloadSeconds[NAMEKEY(name)] = INI::scanReal(ini->getNextToken());
}

//-------------------------------------------------------------------------------------------------
// "VsInfantryByDamage = ARMOR_PIERCING 1000%": the VsInfantry of weapons of that DamageType.
void parseVsInfantryByDamageLine(INI *ini, void * /*instance*/, void * /*store*/, const void * /*userData*/)
{
	const char *name = ini->getNextToken();
	const Int type = DamageTypeFlags::getSingleBitFromName(name);
	if (type < 0)
	{
		DEBUG_CRASH(("IbmEconomy: unknown DamageType '%s'", name));
		throw INI_INVALID_DATA;
	}
	s_vsInfantryByDamage[type] = INI::scanPercentToReal(ini->getNextToken());
}

const FieldParse s_fieldParse[] =
{
	{ "Kind",	parseKindLine,		nullptr, 0 },
	{ "Exempt",	parseExemptLine,	nullptr, 0 },
	{ "Unit",	parseUnitLine,		nullptr, 0 },
	{ "Reload",	parseReloadLine,	nullptr, 0 },
	{ "VsInfantryByDamage",	parseVsInfantryByDamageLine,	nullptr, 0 },
	{ nullptr, nullptr, nullptr, 0 }
};

//-------------------------------------------------------------------------------------------------
// The Unit line for this name, or for the name without its general's prefix ("Tank_", "GC_Chem_").
const EconomyScales *findUnitLine(const AsciiString &templateName)
{
	const char *name = templateName.str();
	while (name != nullptr && *name != '\0')
	{
		std::map<NameKeyType, EconomyScales>::const_iterator it = s_units.find(NAMEKEY(name));
		if (it != s_units.end())
			return &it->second;

		name = strchr(name, '_');
		if (name != nullptr)
			++name;
	}
	return nullptr;
}

//-------------------------------------------------------------------------------------------------
const EconomyScales &getScales(const ThingTemplate *tmpl)
{
	std::map<const ThingTemplate *, EconomyScales>::iterator cached = s_cache.find(tmpl);
	if (cached != s_cache.end())
		return cached->second;

	EconomyScales scales(1.0f);

	if (tmpl->friend_getBuildCost() > 0 && !tmpl->isAnyKindOf(s_exempt))
	{
		for (size_t i = 0; i < s_kinds.size(); ++i)
		{
			if (tmpl->isKindOf(s_kinds[i].kind))
			{
				for (Int stat = 0; stat < EconomyStat_Count; ++stat)
					if (s_kinds[i].scales.scale[stat] >= 0.0f)
						scales.scale[stat] = s_kinds[i].scales.scale[stat];
				break;
			}
		}
	}

	const EconomyScales *unit = findUnitLine(tmpl->getName());
	if (unit != nullptr)
	{
		for (Int stat = 0; stat < EconomyStat_Count; ++stat)
			if (unit->scale[stat] >= 0.0f)
				scales.scale[stat] = unit->scale[stat];
	}

	return s_cache.insert(std::make_pair(tmpl, scales)).first->second;
}

} // namespace

//-------------------------------------------------------------------------------------------------
void IbmEngine::parseEconomyDefinition(INI *ini)
{
	// Each game's pack sets everything again.
	s_kinds.clear();
	s_exempt.clear();
	s_units.clear();
	s_cache.clear();
	s_reloadSeconds.clear();
	s_reloadCache.clear();
	s_vsInfantryByDamage.clear();
	Int unused = 0;
	ini->initFromINI(&unused, s_fieldParse);
}

//-------------------------------------------------------------------------------------------------
Real IbmEngine::getEconomyScale(const ThingTemplate *tmpl, EconomyStat stat)
{
	if (!isFeatureActive(Feature_Economy) || tmpl == nullptr)
		return 1.0f;
	return getScales(tmpl).scale[stat];
}

//-------------------------------------------------------------------------------------------------
Int IbmEngine::getEconomyReloadFrames(const WeaponTemplate *weapon)
{
	if (!isFeatureActive(Feature_Economy) || weapon == nullptr || s_reloadSeconds.empty())
		return -1;

	std::map<const WeaponTemplate *, Int>::const_iterator cached = s_reloadCache.find(weapon);
	if (cached != s_reloadCache.end())
		return cached->second;

	// Its own line, or the line of the weapon it is a general's version of ("Nuke_" + name).
	Int frames = -1;
	const char *name = weapon->getName().str();
	while (name != nullptr && *name != '\0')
	{
		std::map<NameKeyType, Real>::const_iterator it = s_reloadSeconds.find(NAMEKEY(name));
		if (it != s_reloadSeconds.end())
		{
			frames = REAL_TO_INT_CEIL(it->second * LOGICFRAMES_PER_SECOND);
			break;
		}

		name = strchr(name, '_');
		if (name != nullptr)
			++name;
	}

	s_reloadCache[weapon] = frames;
	return frames;
}

//-------------------------------------------------------------------------------------------------
Real IbmEngine::getEconomyDamageScaleAgainst(const WeaponTemplate *weapon, const Object *attacker, const Object *victim)
{
	if (attacker == nullptr || victim == nullptr || !victim->isKindOf(KINDOF_INFANTRY))
		return 1.0f;

	const Real scale = getEconomyScale(attacker->getTemplate(), EconomyStat_VsInfantry);
	if (scale == 1.0f || weapon == nullptr)
		return scale;

	std::map<Int, Real>::const_iterator it = s_vsInfantryByDamage.find(weapon->getDamageType());
	return it != s_vsInfantryByDamage.end() ? it->second : scale;
}

//-------------------------------------------------------------------------------------------------
void IbmEngine::applyEconomySpeed(const Object *obj, LocomotorSet &set)
{
	const Real scale = getEconomyScale(obj->getTemplate(), EconomyStat_Speed);
	if (scale == 1.0f)
		return;

	// A set holds a locomotor for each surface it can cross; a locomotor may cover several.
	static const LocomotorSurfaceTypeMask surfaces[] =
	{
		LOCOMOTORSURFACE_GROUND, LOCOMOTORSURFACE_WATER, LOCOMOTORSURFACE_CLIFF, LOCOMOTORSURFACE_AIR, LOCOMOTORSURFACE_RUBBLE
	};
	Locomotor *done[ARRAY_SIZE(surfaces)];
	size_t doneCount = 0;
	for (size_t i = 0; i < ARRAY_SIZE(surfaces); ++i)
	{
		Locomotor *loco = set.findLocomotor(surfaces[i]);
		if (loco == nullptr || std::find(done, done + doneCount, loco) != done + doneCount)
			continue;

		// A new locomotor has no cap yet, so this is its normal speed.
		loco->setMaxSpeed(loco->getMaxSpeedForCondition(BODY_PRISTINE) * scale);
		done[doneCount++] = loco;
	}
}
