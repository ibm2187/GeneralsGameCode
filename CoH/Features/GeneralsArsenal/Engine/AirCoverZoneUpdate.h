// CoH GeneralsArsenal: the module on an air cover zone. An air cover power drops the zone on its
// target (an OCL); the zone calls jets in from the nearest map edge, keeps them over the area for a
// while attacking what enters (aircraft for air superiority, vehicles for ground attack), then sends
// them back off the map. The jets fight with their own weapons; anti-air can shoot them down.
//
//     Behavior = AirCoverZoneUpdate ModuleTag_CoHCover
//       Aircraft    = CoHCover_AmericaJetRaptor
//       Count       = 2
//       Duration    = 45000           ; ms over the area
//       Radius      = 350             ; what enters this is attacked
//       TargetKinds = AIRCRAFT        ; any of these
//     End

#pragma once

#include "Common/KindOf.h"
#include "GameLogic/Module/UpdateModule.h"

#include <vector>

class AirCoverZoneUpdateModuleData : public UpdateModuleData
{
public:
	AsciiString m_aircraftName;
	Int m_count;
	UnsignedInt m_durationFrames;
	Real m_radius;
	KindOfMaskType m_targetKinds;

	AirCoverZoneUpdateModuleData();
	static void buildFieldParse( MultiIniFieldParse &p );
};

class AirCoverZoneUpdate : public UpdateModule
{
	MAKE_STANDARD_MODULE_MACRO_WITH_MODULE_DATA( AirCoverZoneUpdate, AirCoverZoneUpdateModuleData );
	MEMORY_POOL_GLUE_WITH_EXPLICIT_CREATE( AirCoverZoneUpdate, "AirCoverZoneUpdate", 16, 16 )	// not in the engine's pool table

public:
	AirCoverZoneUpdate( Thing *thing, const ModuleData *moduleData );
	// virtual destructor prototype provided by memory pool declaration

	virtual UpdateSleepTime update() override;

	Real getRadius() const { return getAirCoverZoneUpdateModuleData()->m_radius; }
	Bool isPatrolling() const { return m_phase == PHASE_PATROLLING; }

	Object *ownerObject() { return getObject(); }	///< for the UI

	static AirCoverZoneUpdate *find( Object *object );

private:
	enum Phase
	{
		PHASE_CALLING,
		PHASE_PATROLLING,
		PHASE_LEAVING,
	};

	void callAircraft();
	void forgetLostAircraft();
	void patrol();
	void leave();
	Object *findTarget( const Object *jet ) const;
	Bool isWorthKeeping( const Object *target ) const;
	Coord3D orbitPoint( Int index ) const;

	Int m_phase;
	UnsignedInt m_startFrame;
	UnsignedInt m_leaveFrame;
	Int m_orbitStep;
	std::vector<ObjectID> m_aircraft;
};
