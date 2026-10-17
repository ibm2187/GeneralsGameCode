// CoH Sectors: the module on each sector point object (CoHSectorPoint in the pack). It holds what the
// point is and its capture state, so a save game and the CRC check carry them like any object's state.
// The logic itself runs once per frame for all points (World.cpp), not in this module's update.

#pragma once

#include "GameLogic/Module/UpdateModule.h"

#include "Features/Sectors/Domain/Capture.h"

#include <vector>

class SectorPointUpdate : public UpdateModule
{
	MAKE_STANDARD_MODULE_MACRO( SectorPointUpdate );
	MEMORY_POOL_GLUE_WITH_EXPLICIT_CREATE( SectorPointUpdate, "SectorPointUpdate", 32, 32 )	// not in the engine's pool table

public:
	SectorPointUpdate( Thing *thing, const ModuleData *moduleData );
	// virtual destructor prototype provided by memory pool declaration

	virtual UpdateSleepTime update() override { return UPDATE_SLEEP_FOREVER; }

	void setUp( CoH::Sectors::SectorKind kind, CoH::Sectors::PlayerId homePlayer, const std::vector<ObjectID> &members );

	CoH::Sectors::SectorKind getKind() const { return m_kind; }
	CoH::Sectors::PlayerId getHomePlayer() const { return m_homePlayer; }
	const std::vector<ObjectID> &getMembers() const { return m_members; }

	CoH::Sectors::PointState &state() { return m_state; }
	const CoH::Sectors::PointState &state() const { return m_state; }

	// The point module of a sector point object, or null for any other object.
	static SectorPointUpdate *find( Object *object );

private:
	CoH::Sectors::SectorKind m_kind;
	CoH::Sectors::PlayerId m_homePlayer;
	std::vector<ObjectID> m_members;	///< the map buildings the point was made from
	CoH::Sectors::PointState m_state;
};
