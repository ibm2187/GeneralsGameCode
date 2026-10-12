// IBM Engine: IbmSquadUpdate, the module that makes a mob leader a squad leader.
//
//   Behavior = IbmSquadUpdate IbmSquadTag_Squad
//     AIReplaces = AmericaInfantryRanger   ; AI teams that ask for this unit build the squad instead
//     StealthWithMembers = Yes             ; the leader is stealthed only while all members are hidden
//     ReinforceAt = AmericaBarracks        ; buildings of the squad's owner that bring back lost members
//     ReinforceRadius = 200                ; how close the leader must be to one of them
//     ReinforceDelay = 2000                ; one member back per this many milliseconds
//   End
//
// It keeps track of members that follow their own order (a skill, entering a container), so the
// mob code doesn't pull them back to the leader, parks the leader inside the container its members
// ride in, and brings back lost members near its Barracks (SquadReinforce.cpp).

#pragma once

#include "GameLogic/Module/UpdateModule.h"

#include "Features/Squads/SquadsInternal.h"

//-------------------------------------------------------------------------------------------------
class IbmSquadUpdateModuleData : public UpdateModuleData
{
public:
	AsciiString m_aiReplaces;		///< basic infantry unit this squad replaces in AI teams
	Bool m_stealthWithMembers;		///< leader may stealth only while every member is stealthed and undetected
	std::vector<AsciiString> m_reinforceAt;	///< buildings that bring back lost members
	Real m_reinforceRadius;			///< how close the leader must be to one of them
	UnsignedInt m_reinforceDelay;	///< frames between two members coming back

	IbmSquadUpdateModuleData();
	static void buildFieldParse(MultiIniFieldParse& p);
};

//-------------------------------------------------------------------------------------------------
class IbmSquadUpdate : public UpdateModule
{
	IBM_MEMORY_POOL_GLUE( IbmSquadUpdate, "IbmSquadUpdate", 64, 64 )
	MAKE_STANDARD_MODULE_MACRO_WITH_MODULE_DATA( IbmSquadUpdate, IbmSquadUpdateModuleData )

public:
	IbmSquadUpdate( Thing *thing, const ModuleData* moduleData );
	// virtual destructor prototype provided by memory pool declaration

	virtual UpdateSleepTime update() override;
	// Runs while parked, when the leader is held.
	virtual DisabledMaskType getDisabledTypesToProcess() const override { return DISABLEDMASK_ALL; }

	const IbmSquadUpdateModuleData *getData() const { return getIbmSquadUpdateModuleData(); }

	/// The member follows an order of its own until it is idle again.
	void setMemberOnOwnOrder( ObjectID member );

	/// TRUE while the member follows its own order or rides inside a container.
	Bool isMemberOnOwnOrder( Object *member );

	/// TRUE while all members ride inside a container and the leader rides along with them.
	Bool isParked() const { return m_parked; }

private:
	void park( Object *container );
	void unpark( const Coord3D *pos );

	std::list<ObjectID> m_ownOrderMembers;	///< members following their own order
	Bool m_parked;							///< all members are inside a container
	UnsignedInt m_nextReinforceFrame;		///< when to look for a Barracks again
};
