// CoH GeneralsArsenal: on a called-in plane (a sortie or an air cover jet). When it is shot down, the
// power that sent it is ready later by that power's LossPenalty, so air defense and escorts matter.
// A plane that leaves the map isn't lost: it is removed without dying. A plane several powers use
// (the B-52 of the carpet bomb and the daisy cutter) is told which one sent it as it is made.
//
//     Behavior = SortieLossDie ModuleTag_CoHLoss
//       SpecialPower = CoHUSA_RaptorStrike
//     End

#pragma once

#include "GameLogic/Module/DieModule.h"

class SpecialPowerTemplate;

class SortieLossDieModuleData : public DieModuleData
{
public:
	const SpecialPowerTemplate *m_specialPower;

	SortieLossDieModuleData() : m_specialPower( nullptr ) {}

	static void buildFieldParse( MultiIniFieldParse &p );
};

class SortieLossDie : public DieModule
{
	MAKE_STANDARD_MODULE_MACRO_WITH_MODULE_DATA( SortieLossDie, SortieLossDieModuleData );
	MEMORY_POOL_GLUE_WITH_EXPLICIT_CREATE( SortieLossDie, "SortieLossDie", 64, 64 )	// not in the engine's pool table

public:
	SortieLossDie( Thing *thing, const ModuleData *moduleData );
	// virtual destructor prototype provided by memory pool declaration

	virtual void onDie( const DamageInfo *damageInfo ) override;

	void setSpecialPower( const SpecialPowerTemplate *power ) { m_power = power; }

	static SortieLossDie *find( Object *object );

private:
	const SpecialPowerTemplate *m_power;	///< the power that sent the plane
};
