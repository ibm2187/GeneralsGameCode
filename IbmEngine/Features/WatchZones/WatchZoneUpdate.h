// IBM Engine: IbmWatchZoneUpdate, the module of a watch zone object. A building's power creates the
// zone at the spot the player picks (OCLSpecialPower, CREATE_AT_LOCATION).
//
//   Behavior = IbmWatchZoneUpdate IbmWatchZoneTag_Zone
//     Radius = 175                   ; enemies inside this circle are attacked
//     Duration = 40000               ; how long the zone watches, in milliseconds
//     TargetKindOf = AIRCRAFT        ; enemies of any of these kinds...
//     ForbiddenKindOf = STRUCTURE    ; ...and none of these
//     Targets = AIRBORNE             ; ANY, AIRBORNE (in the air now) or GROUND (on the ground now)
//     Strikes = 3                    ; passes it calls in at most; the zone ends after the last one
//     JetsPerStrike = 2              ; jets in each pass, side by side, each at its own enemy (default 1)
//     JetSpacing = 40                ; how far apart they fly
//     StrikeDelay = 6000             ; at least this long between two passes, in milliseconds
//     StrikeOCL = IbmOCL_RaptorPass  ; a DeliverPayload with one transport: the jet
//     ApproachDistance = 900         ; the jet appears this far from the enemy, on the side of the
//                                    ; map edge nearest the building that made the zone (0: at that edge)
//     FollowRange = 300              ; the jet follows its enemy while it stays this close to the zone
//     ZoneDecal                      ; the circle on the ground (like DeliverPayload's DeliveryDecal)
//       ...
//     End
//   End
//
// Each jet goes at the nearest enemy that no other jet of the zone is after.

#pragma once

#include "GameClient/RadiusDecal.h"
#include "GameLogic/Module/UpdateModule.h"

#include "Features/FeatureFlags/MemoryPoolGlue.h"

class ObjectCreationList;

//-------------------------------------------------------------------------------------------------
class IbmWatchZoneUpdateModuleData : public UpdateModuleData
{
public:
	enum TargetHeight
	{
		TARGETS_ANY,
		TARGETS_AIRBORNE,
		TARGETS_GROUND,
	};

	Real m_radius;
	UnsignedInt m_duration;					///< frames
	KindOfMaskType m_targetKindOf;
	KindOfMaskType m_forbiddenKindOf;
	Int m_targets;							///< TargetHeight
	Int m_strikes;
	Int m_jetsPerStrike;
	Real m_jetSpacing;
	UnsignedInt m_strikeDelay;				///< frames
	const ObjectCreationList *m_strikeOCL;
	Real m_approachDistance;
	Real m_followRange;
	RadiusDecalTemplate m_zoneDecal;

	IbmWatchZoneUpdateModuleData();
	static void buildFieldParse(MultiIniFieldParse& p);
};

//-------------------------------------------------------------------------------------------------
class IbmWatchZoneUpdate : public UpdateModule
{
	IBM_MEMORY_POOL_GLUE( IbmWatchZoneUpdate, "IbmWatchZoneUpdate", 16, 16 )
	MAKE_STANDARD_MODULE_MACRO_WITH_MODULE_DATA( IbmWatchZoneUpdate, IbmWatchZoneUpdateModuleData )

public:
	IbmWatchZoneUpdate( Thing *thing, const ModuleData* moduleData );
	// virtual destructor prototype provided by memory pool declaration

	virtual UpdateSleepTime update() override;

	const IbmWatchZoneUpdateModuleData *getData() const { return getIbmWatchZoneUpdateModuleData(); }

private:
	Bool isTarget( const Object *obj ) const;
	Bool isJetAfter( ObjectID target ) const;
	Object *findTarget() const;
	Bool callStrike( Object *target, Real sideOffset );

	Bool m_started;							///< the first update set up the zone
	UnsignedInt m_endFrame;
	UnsignedInt m_nextStrikeFrame;
	Int m_strikesLeft;
	Coord3D m_home;							///< map edge point the jets come from
	std::list<ObjectID> m_jets;				///< this zone's jets still in the air
	RadiusDecal m_decal;					///< shown only, so not saved; made again after a load
};
