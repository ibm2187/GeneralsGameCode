// CoH VehicleMovement: see Config.h.

#include "PreRTS.h"	// This must go first in EVERY cpp file in the GameEngine

#include "Common/INI.h"
#define DEFINE_TERRAIN_TYPE_NAMES	// the class names, for Ground lines
#include "Common/TerrainTypes.h"

#include "Kernel/Ini.h"

#include "Features/VehicleMovement/Engine/Config.h"

namespace CoH::VehicleMovement
{

Config::Config()
{
	vehicles.set(KINDOF_VEHICLE);
	exempt.set(KINDOF_AIRCRAFT);
}

namespace
{

Config &storage()
{
	static Config theConfig;
	return theConfig;
}

Config &configOf(void *instance)
{
	return *static_cast<Config *>(instance);
}

// Class = <name> <tons>
void parseClass(INI *ini, void *instance, void *, const void *)
{
	WeightClass line;
	line.name = ini->getNextToken();
	line.tons = INI::scanReal(ini->getNextToken());
	configOf(instance).classes.push_back(line);
}

bool isNumber(const char *token)
{
	return (token[0] >= '0' && token[0] <= '9') || token[0] == '.';
}

// Unit = <object> <class or tons>
void parseUnit(INI *ini, void *instance, void *, const void *)
{
	UnitLine line;
	line.name = ini->getNextToken();
	const char *weight = ini->getNextToken();
	if (isNumber(weight))
		line.tons = INI::scanReal(weight);
	else
		line.weightClass = weight;
	configOf(instance).units.push_back(line);
}

// Upgrade = <upgrade> <tons>
void parseUpgrade(INI *ini, void *instance, void *, const void *)
{
	UpgradeLine line;
	line.name = ini->getNextToken();
	line.tons = INI::scanReal(ini->getNextToken());
	configOf(instance).upgrades.push_back(line);
}

// Surface = <name> <wheels percent> <treads percent>
void parseSurface(INI *ini, void *instance, void *, const void *)
{
	SurfaceLine line;
	line.name = ini->getNextToken();
	line.speed.wheels = INI::scanPercentToReal(ini->getNextToken());
	line.speed.treads = INI::scanPercentToReal(ini->getNextToken());
	configOf(instance).surfaces.push_back(line);
}

// Ground = <terrain class> <surface>
void parseGround(INI *ini, void *instance, void *, const void *)
{
	GroundLine line;
	line.terrain = static_cast<TerrainClass>(INI::scanIndexList(ini->getNextToken(), terrainTypeNames));
	line.surface = ini->getNextToken();
	configOf(instance).grounds.push_back(line);
}

// Tile = <start of the terrain texture's name> <surface>
void parseTile(INI *ini, void *instance, void *, const void *)
{
	TileLine line;
	line.prefix = ini->getNextToken();
	line.surface = ini->getNextToken();
	configOf(instance).tiles.push_back(line);
}

// Road = <start of the road's name> <surface>
void parseRoad(INI *ini, void *instance, void *, const void *)
{
	RoadLine line;
	line.prefix = ini->getNextToken();
	line.surface = ini->getNextToken();
	configOf(instance).roads.push_back(line);
}

// <Curve> = <tons> <percent> <tons> <percent> ...
template <Curve HandlingCurves::*member>
void parseCurve(INI *ini, void *instance, void *, const void *)
{
	Curve &curve = configOf(instance).curves.*member;
	curve.clear();
	while (const char *weight = ini->getNextTokenOrNull())
	{
		const float tons = INI::scanReal(weight);
		curve.add({ tons, INI::scanPercentToReal(ini->getNextToken()) });
	}
}

} // namespace

void parseConfigBlock(INI *ini)
{
	static const Ini::Table<Config> fields = {
		Ini::custom<Config>("Class", parseClass),
		Ini::field("DefaultClass", &Config::defaultClass),
		Ini::custom<Config>("Unit", parseUnit),
		Ini::custom<Config>("Upgrade", parseUpgrade),
		Ini::custom<Config>("Speed", parseCurve<&HandlingCurves::speed>),
		Ini::custom<Config>("Acceleration", parseCurve<&HandlingCurves::acceleration>),
		Ini::custom<Config>("Braking", parseCurve<&HandlingCurves::braking>),
		Ini::custom<Config>("TurnRate", parseCurve<&HandlingCurves::turnRate>),
		Ini::custom<Config>("SlopePenalty", parseCurve<&HandlingCurves::slopePenalty>),
		Ini::percent("SlopeMinSpeed", &Config::curves, &HandlingCurves::slopeMinSpeed),
		Ini::field("SlopeLookAhead", &Config::slopeLookAhead),
		Ini::field("Vehicles", &Config::vehicles),
		Ini::field("Exempt", &Config::exempt),
		Ini::custom<Config>("Surface", parseSurface),
		Ini::custom<Config>("Tile", parseTile),
		Ini::custom<Config>("Ground", parseGround),
		Ini::custom<Config>("Road", parseRoad),
		Ini::field("DefaultSurface", &Config::defaultSurface),
		Ini::field("BridgeSurface", &Config::bridgeSurface),
	};

	// The block starts from the defaults: its Class, Unit, Upgrade, Surface, Tile, Ground and Road lines are
	// the whole list.
	Config &c = storage();
	c = Config();
	fields.read(ini, c);
}

const Config &config()
{
	return storage();
}

void resetConfig()
{
	storage() = Config();
}

std::uint8_t surfaceIndex(const AsciiString &name)
{
	const std::vector<SurfaceLine> &surfaces = storage().surfaces;
	for (size_t i = 0; i < surfaces.size() && i < SurfaceGrid::NO_SURFACE; ++i)
	{
		if (surfaces[i].name.compareNoCase(name) == 0)
			return static_cast<std::uint8_t>(i);
	}
	return SurfaceGrid::NO_SURFACE;
}

float classTons(const AsciiString &name)
{
	const Config &c = storage();
	for (const WeightClass &line : c.classes)
	{
		if (line.name.compareNoCase(name) == 0)
			return line.tons;
	}
	for (const WeightClass &line : c.classes)
	{
		if (line.name.compareNoCase(c.defaultClass) == 0)
			return line.tons;
	}
	return 0.0f;	// no Class lines at all: every vehicle sits at the curves' first point
}

} // namespace CoH::VehicleMovement
