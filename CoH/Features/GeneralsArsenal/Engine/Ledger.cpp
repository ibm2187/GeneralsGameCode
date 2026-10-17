// CoH GeneralsArsenal: the ledger, one hidden neutral object whose module keeps every player's command
// points. A new game creates it; a loaded game brings it back with its objects. Its module registers
// itself, so the ledger is found the same way in both.

#include "PreRTS.h"	// This must go first in EVERY cpp file in the GameEngine

#include "Common/Player.h"
#include "Common/PlayerList.h"
#include "Common/ThingFactory.h"
#include "Common/ThingTemplate.h"
#include "GameLogic/Object.h"

#include "Kernel/Log.h"

#include "Features/GeneralsArsenal/Engine/CommandPointsLedger.h"
#include "Features/GeneralsArsenal/Engine/Config.h"
#include "Features/GeneralsArsenal/Engine/Internal.h"

namespace CoH::GeneralsArsenal
{

namespace
{

CommandPointsLedger *&theLedger()
{
	static CommandPointsLedger *ledger = nullptr;
	return ledger;
}

} // namespace

void createLedger()
{
	const ThingTemplate *ledgerTemplate = TheThingFactory->findTemplate(config().ledgerObject);
	if (ledgerTemplate == nullptr)
	{
		CoH::log("GeneralsArsenal: no '%s' object in the pack; no command points", config().ledgerObject.str());
		return;
	}
	Object *object = TheThingFactory->newObject(ledgerTemplate, ThePlayerList->getNeutralPlayer()->getDefaultTeam());
	if (object == nullptr)
		return;
	Coord3D corner = { 1.0f, 1.0f, 0.0f };
	object->setPosition(&corner);
}

void registerLedger(CommandPointsLedger *ledger)
{
	theLedger() = ledger;
}

void unregisterLedger(CommandPointsLedger *ledger)
{
	if (theLedger() == ledger)
		theLedger() = nullptr;
}

Wallet *walletOf(const Player *player)
{
	CommandPointsLedger *ledger = theLedger();
	if (ledger == nullptr || player == nullptr)
		return nullptr;
	return &ledger->wallet(player->getPlayerIndex());
}

int pointsOf(const Player *player)
{
	const Wallet *wallet = walletOf(player);
	return wallet ? wallet->whole() : 0;
}

} // namespace CoH::GeneralsArsenal
