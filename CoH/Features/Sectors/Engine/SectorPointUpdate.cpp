// CoH Sectors: see SectorPointUpdate.h.

#include "PreRTS.h"	// This must go first in EVERY cpp file in the GameEngine

#include "Common/Xfer.h"
#include "GameLogic/Object.h"

#include "Features/Sectors/Engine/SectorPointUpdate.h"

using namespace CoH::Sectors;

SectorPointUpdate::SectorPointUpdate( Thing *thing, const ModuleData *moduleData ) :
	UpdateModule( thing, moduleData ),
	m_kind( SectorKind::Supply ),
	m_homePlayer( NoPlayer )
{
	setWakeFrame( getObject(), UPDATE_SLEEP_FOREVER );
}

SectorPointUpdate::~SectorPointUpdate()
{
}

void SectorPointUpdate::setUp( SectorKind kind, PlayerId homePlayer, const std::vector<ObjectID> &members )
{
	m_kind = kind;
	m_homePlayer = homePlayer;
	m_members = members;
	m_state = PointState();
	if( kind == SectorKind::Home )
		m_state = { homePlayer, homePlayer, 1.0f };
}

SectorPointUpdate *SectorPointUpdate::find( Object *object )
{
	static const NameKeyType key = NAMEKEY( "SectorPointUpdate" );
	return object ? (SectorPointUpdate *)object->findUpdateModule( key ) : nullptr;
}

// The state is part of the CRC, so a game whose capture went different ways is caught as a mismatch.
void SectorPointUpdate::crc( Xfer *xfer )
{
	SectorPointUpdate::xfer( xfer );
}

// ------------------------------------------------------------------------------------------------
/** Xfer method
	* Version Info:
	* 1: Initial version */
// ------------------------------------------------------------------------------------------------
void SectorPointUpdate::xfer( Xfer *xfer )
{
	XferVersion currentVersion = 1;
	XferVersion version = currentVersion;
	xfer->xferVersion( &version, currentVersion );

	UpdateModule::xfer( xfer );

	Int kind = (Int)m_kind;
	xfer->xferInt( &kind );
	m_kind = (SectorKind)kind;
	xfer->xferInt( &m_homePlayer );

	UnsignedShort memberCount = (UnsignedShort)m_members.size();
	xfer->xferUnsignedShort( &memberCount );
	m_members.resize( memberCount );
	for( ObjectID &member : m_members )
		xfer->xferObjectID( &member );

	xfer->xferInt( &m_state.owner );
	xfer->xferInt( &m_state.progressSide );
	xfer->xferReal( &m_state.progress );
}

void SectorPointUpdate::loadPostProcess()
{
	UpdateModule::loadPostProcess();
}
