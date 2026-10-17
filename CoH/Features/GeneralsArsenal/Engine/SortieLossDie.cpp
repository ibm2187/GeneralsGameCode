// CoH GeneralsArsenal: see SortieLossDie.h.

#include "PreRTS.h"	// This must go first in EVERY cpp file in the GameEngine

#include "Common/INI.h"
#include "Common/SpecialPower.h"
#include "Common/Xfer.h"
#include "GameLogic/Object.h"

#include "Features/GeneralsArsenal/Engine/Internal.h"
#include "Features/GeneralsArsenal/Engine/SortieLossDie.h"

void SortieLossDieModuleData::buildFieldParse( MultiIniFieldParse &p )
{
	DieModuleData::buildFieldParse( p );

	static const FieldParse dataFieldParse[] =
	{
		{ "SpecialPower", INI::parseSpecialPowerTemplate, nullptr, offsetof( SortieLossDieModuleData, m_specialPower ) },
		{ nullptr, nullptr, nullptr, 0 }
	};
	p.add( dataFieldParse );
}

SortieLossDie::SortieLossDie( Thing *thing, const ModuleData *moduleData ) :
	DieModule( thing, moduleData ),
	m_power( getSortieLossDieModuleData()->m_specialPower )
{
}

SortieLossDie *SortieLossDie::find( Object *object )
{
	static const NameKeyType key = NAMEKEY( "SortieLossDie" );
	for( BehaviorModule **module = object->getBehaviorModules(); *module; ++module )
	{
		if( (*module)->getModuleNameKey() == key )
			return static_cast<SortieLossDie *>( *module );
	}
	return nullptr;
}

SortieLossDie::~SortieLossDie()
{
}

void SortieLossDie::onDie( const DamageInfo *damageInfo )
{
	if( !isDieApplicable( damageInfo ) )
		return;
	Object *plane = getObject();
	CoH::GeneralsArsenal::penalizeLoss( plane->getControllingPlayer(), m_power, plane );
}

void SortieLossDie::crc( Xfer *xfer )
{
	DieModule::crc( xfer );
}

// ------------------------------------------------------------------------------------------------
/** Xfer method
	* Version Info:
	* 1: Initial version */
// ------------------------------------------------------------------------------------------------
void SortieLossDie::xfer( Xfer *xfer )
{
	XferVersion currentVersion = 1;
	XferVersion version = currentVersion;
	xfer->xferVersion( &version, currentVersion );
	DieModule::xfer( xfer );

	AsciiString power = m_power ? m_power->getName() : AsciiString::TheEmptyString;
	xfer->xferAsciiString( &power );
	if( xfer->getXferMode() == XFER_LOAD )
		m_power = power.isEmpty() ? nullptr : TheSpecialPowerStore->findSpecialPowerTemplate( power );
}

void SortieLossDie::loadPostProcess()
{
	DieModule::loadPostProcess();
}
