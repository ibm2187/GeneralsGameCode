// CoH GeneralsArsenal: the module on the hidden ledger object (CoHCommandLedger in the pack), one per
// game. It holds every player's command points, so a save game carries them like any
// object's state.

#pragma once

#include "GameLogic/Module/UpdateModule.h"

#include "Features/GeneralsArsenal/Domain/CommandPoints.h"

class CommandPointsLedger : public UpdateModule
{
	MAKE_STANDARD_MODULE_MACRO( CommandPointsLedger );
	MEMORY_POOL_GLUE_WITH_EXPLICIT_CREATE( CommandPointsLedger, "CommandPointsLedger", 4, 4 )	// not in the engine's pool table

public:
	CommandPointsLedger( Thing *thing, const ModuleData *moduleData );
	// virtual destructor prototype provided by memory pool declaration

	virtual UpdateSleepTime update() override { return UPDATE_SLEEP_FOREVER; }

	CoH::GeneralsArsenal::Wallet &wallet( Int playerIndex );

	// The ledger module of an object, or null for any other object.
	static CommandPointsLedger *find( Object *object );

private:
	CoH::GeneralsArsenal::Wallet m_wallets[ MAX_PLAYER_COUNT ];
};
