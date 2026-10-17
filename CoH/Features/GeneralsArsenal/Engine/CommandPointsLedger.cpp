// CoH GeneralsArsenal: see CommandPointsLedger.h.

#include "PreRTS.h"	// This must go first in EVERY cpp file in the GameEngine

#include "Common/Xfer.h"
#include "GameLogic/Object.h"

#include "Features/GeneralsArsenal/Engine/CommandPointsLedger.h"
#include "Features/GeneralsArsenal/Engine/Internal.h"

using namespace CoH::GeneralsArsenal;

CommandPointsLedger::CommandPointsLedger( Thing *thing, const ModuleData *moduleData ) :
	UpdateModule( thing, moduleData )
{
	setWakeFrame( getObject(), UPDATE_SLEEP_FOREVER );
	CoH::GeneralsArsenal::registerLedger( this );
}

CommandPointsLedger::~CommandPointsLedger()
{
	CoH::GeneralsArsenal::unregisterLedger( this );
}

Wallet &CommandPointsLedger::wallet( Int playerIndex )
{
	static Wallet nobody;
	if( playerIndex < 0 || playerIndex >= MAX_PLAYER_COUNT )
	{
		nobody = Wallet();
		return nobody;
	}
	return m_wallets[ playerIndex ];
}

CommandPointsLedger *CommandPointsLedger::find( Object *object )
{
	static const NameKeyType key = NAMEKEY( "CommandPointsLedger" );
	return object ? (CommandPointsLedger *)object->findUpdateModule( key ) : nullptr;
}

// The engine's object CRC doesn't include modules; this is here for a CRC that does.
void CommandPointsLedger::crc( Xfer *xfer )
{
	CommandPointsLedger::xfer( xfer );
}

// ------------------------------------------------------------------------------------------------
/** Xfer method
	* Version Info:
	* 1: Initial version */
// ------------------------------------------------------------------------------------------------
void CommandPointsLedger::xfer( Xfer *xfer )
{
	XferVersion currentVersion = 1;
	XferVersion version = currentVersion;
	xfer->xferVersion( &version, currentVersion );

	UpdateModule::xfer( xfer );

	for( Wallet &wallet : m_wallets )
	{
		Real points = wallet.points();
		xfer->xferReal( &points );
		wallet.set( points );
	}
}

void CommandPointsLedger::loadPostProcess()
{
	UpdateModule::loadPostProcess();
}
