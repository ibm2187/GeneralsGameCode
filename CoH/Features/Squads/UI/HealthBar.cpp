// CoH Squads: a squad shows one health bar, the whole squad's, with a pip under it for each member
// it was made with (dark for the lost ones). The leader shows it; while the leader isn't selected,
// the member under the cursor does.

#include "PreRTS.h"	// This must go first in EVERY cpp file in the GameEngine

#include "GameClient/Color.h"
#include "GameClient/Display.h"
#include "GameClient/Drawable.h"
#include "GameClient/InGameUI.h"
#include "GameLogic/Module/BodyModule.h"
#include "GameLogic/Object.h"

#include "Features/Squads/Engine/Internal.h"

namespace CoH::Squads
{

namespace
{

bool isShowingBar(const Object *object)
{
	const Drawable *drawable = object->getDrawable();
	if (drawable == nullptr)
		return false;
	return drawable->isSelected() || (TheInGameUI != nullptr && TheInGameUI->getMousedOverDrawableID() == drawable->getID());
}

void drawMemberPips(const IRegion2D *bar, int slots, int alive)
{
	const Int size = 3;
	const Int gap = 1;
	const Color aliveColor = GameMakeColor(255, 255, 255, 230);
	const Color lostColor = GameMakeColor(60, 60, 60, 200);
	const Int y = bar->hi.y + 2;
	for (int i = 0; i < slots; ++i)
	{
		const Int x = bar->lo.x + i * (size + gap);
		TheDisplay->drawFillRect(x, y, size, size, i < alive ? aliveColor : lostColor);
	}
}

} // namespace

void onHealthBar(const Object *object, const IRegion2D *bar, float &health, float &maxHealth)
{
	const Squad *squad = squadOfObject(object);
	if (squad == nullptr || bar == nullptr)
		return;
	const std::vector<Object *> living = livingMembers(*squad);
	if (living.empty())
		return;

	// One bar per squad: the others draw none.
	const Object *leader = living.front();
	if (object != leader && isShowingBar(leader))
	{
		maxHealth = 0.0f;
		return;
	}

	health = 0.0f;
	for (const Object *member : living)
	{
		if (const BodyModuleInterface *body = member->getBodyModule())
			health += body->getHealth();
	}
	maxHealth = squad->fullHealth();
	drawMemberPips(bar, squad->slotCount(), (int)living.size());
}

} // namespace CoH::Squads
