/*
**	Command & Conquer Generals Zero Hour(tm)
**	Copyright 2025 Electronic Arts Inc.
**
**	This program is free software: you can redistribute it and/or modify
**	it under the terms of the GNU General Public License as published by
**	the Free Software Foundation, either version 3 of the License, or
**	(at your option) any later version.
**
**	This program is distributed in the hope that it will be useful,
**	but WITHOUT ANY WARRANTY; without even the implied warranty of
**	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**	GNU General Public License for more details.
**
**	You should have received a copy of the GNU General Public License
**	along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

////////////////////////////////////////////////////////////////////////////////
//																																						//
//  (c) 2001-2003 Electronic Arts Inc.																				//
//																																						//
////////////////////////////////////////////////////////////////////////////////

// FILE: W3DModuleFactory.cpp /////////////////////////////////////////////////////////////////////
// Author: Colin Day, April 2001
// Desc:	 W3D specific module
///////////////////////////////////////////////////////////////////////////////////////////////////

// INCLUDES ///////////////////////////////////////////////////////////////////////////////////////
#include "W3DDevice/Common/W3DModuleFactory.h"
#include "W3DDevice/GameClient/Module/W3DDebrisDraw.h"
#include "W3DDevice/GameClient/Module/W3DDefaultDraw.h"
#include "W3DDevice/GameClient/Module/W3DDependencyModelDraw.h"
#include "W3DDevice/GameClient/Module/W3DModelDraw.h"
#include "W3DDevice/GameClient/Module/W3DLaserDraw.h"
#include "W3DDevice/GameClient/Module/W3DOverlordTankDraw.h"
#include "W3DDevice/GameClient/Module/W3DOverlordTruckDraw.h"
#include "W3DDevice/GameClient/Module/W3DOverlordAircraftDraw.h"
#include "W3DDevice/GameClient/Module/W3DPoliceCarDraw.h"
#include "W3DDevice/GameClient/Module/W3DProjectileStreamDraw.h"
#include "W3DDevice/GameClient/Module/W3DRopeDraw.h"
#include "W3DDevice/GameClient/Module/W3DSupplyDraw.h"
#include "W3DDevice/GameClient/Module/W3DScienceModelDraw.h"
#include "W3DDevice/GameClient/Module/W3DTankDraw.h"
#include "W3DDevice/GameClient/Module/W3DTruckDraw.h"
#include "W3DDevice/GameClient/Module/W3DTankTruckDraw.h"
#include "W3DDevice/GameClient/Module/W3DTracerDraw.h"
#include "W3DDevice/GameClient/Module/W3DTreeDraw.h"
#include "W3DDevice/GameClient/Module/W3DPropDraw.h"

//-------------------------------------------------------------------------------------------------
/** Initialize method */
//-------------------------------------------------------------------------------------------------
void W3DModuleFactory::init()
{

	// extending functionality
	ModuleFactory::init();

	// add the specific module templates we need for the draw methods
	addModule( W3DDefaultDraw );
	addModule( W3DDebrisDraw );
	addModule( W3DModelDraw );
	addModule( W3DLaserDraw );
	addModule( W3DOverlordTankDraw );
#if !(RTS_GENERALS && RETAIL_COMPATIBLE_CRC)
	addModule( W3DOverlordTruckDraw );
	addModule( W3DOverlordAircraftDraw );
#endif
	addModule( W3DProjectileStreamDraw );
	addModule( W3DPoliceCarDraw );
	addModule( W3DRopeDraw );
	addModule( W3DScienceModelDraw );
	addModule( W3DSupplyDraw );
	addModule( W3DDependencyModelDraw );
	addModule( W3DTankDraw );
	addModule( W3DTruckDraw );
	addModule( W3DTracerDraw );
	addModule( W3DTankTruckDraw );
#if !(RTS_GENERALS && RETAIL_COMPATIBLE_CRC)
	addModule( W3DTreeDraw );
	addModule( W3DPropDraw );
#endif

}

#if RTS_ZEROHOUR
//-------------------------------------------------------------------------------------------------
// CoH fix: a model draw module's data caches its model's bones (projectile launch points, turrets,
// an attach bone) the first time game logic asks, at the scale of the object asking. An object
// template and its overrides share their module data, so bones cached in one game at an override's
// Scale (a CoH pack's, or a map.ini's) were still used in the next game played in the same process,
// and that game played differently (a replay of it mismatched). Each game now starts with no cached
// bones, as the first game after launch does.
//-------------------------------------------------------------------------------------------------
void W3DModuleFactory::reset()
{
	ModuleFactory::reset();

	for (ModuleDataList::const_iterator it = m_moduleDataList.begin(); it != m_moduleDataList.end(); ++it)
	{
		const W3DModelDrawModuleData* data = (*it)->getAsW3DModelDrawModuleData();
		if (data)
			data->clearCachedBones();
	}
}
#endif
