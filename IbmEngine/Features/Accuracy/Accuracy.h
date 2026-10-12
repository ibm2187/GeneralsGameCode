// IBM Engine: accuracy, the functions the engine's hooks call. With the Accuracy feature on,
// direct fire (rifles, machine guns, snipers, tank guns, rockets) rolls to hit. The chance is the
// weapon's base accuracy, raised by the shooter's veterancy and lowered by range, movement and
// cover. A miss lands on the ground near the target. The IbmAccuracy INI block (in the Accuracy
// pack) sets every number.

#pragma once

class INI;
class Object;
class WeaponTemplate;
class WeaponBonus;
struct Coord3D;

namespace IbmEngine
{

// WeaponTemplate::fireWeaponTemplate, before the shot is resolved: rolls to hit. On a miss returns
// TRUE and moves aimPos to a ground point near the victim, where the shot lands instead. Always
// FALSE when the feature is off, for weapons that don't roll, and for targets that are buildings.
Bool rollAccuracyMiss(const Object *source, const Object *victim, const WeaponTemplate *weapon,
	const WeaponBonus &bonus, Coord3D &aimPos);

// INI block "IbmAccuracy": the settings, read from the Accuracy pack at game start.
void parseAccuracyDefinition(INI *ini);

} // namespace IbmEngine
