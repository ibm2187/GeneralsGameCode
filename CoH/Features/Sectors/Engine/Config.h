// CoH Sectors: the tuning, read from the CoHSectors block of the pack (Data/INI/Sectors.ini). Back to
// these defaults after every game.

#pragma once

#include "Common/KindOf.h"

#include "Features/Sectors/Domain/AiTargets.h"
#include "Features/Sectors/Domain/BorderLines.h"
#include "Features/Sectors/Domain/BuildRule.h"
#include "Features/Sectors/Domain/Capture.h"
#include "Features/Sectors/Domain/Network.h"
#include "Features/Sectors/Domain/Points.h"
#include "Features/Sectors/Domain/SectorMap.h"

#include <vector>

class INI;

namespace CoH::Sectors
{

// In AI script conditions, a supply building's name with 'from' replaced by 'to' (SupplyCenter ->
// Barracks), when such a template exists.
struct ScriptRename
{
	AsciiString from;
	AsciiString to;
};

struct Config
{
	PointRules points;
	MapRules map;
	CaptureRules capture;
	IncomeValues income;
	BuildRules build;
	AiWeights ai;
	LineRules borderLines;				///< how borders are drawn: rounding, point spacing

	Real borderBandWidth = 40.0f;		///< the soft band inside an owned sector's border, world units

	Real captureRadius = 150.0f;		///< units this close to a point capture it
	Real captureIntervalSeconds = 0.5f;	///< how often capture is worked out
	Real incomeSeconds = 10.0f;			///< each held, connected sector pays this often

	KindOfMaskType captureKinds;		///< units of any of these kinds capture...
	KindOfMaskType nonCaptureKinds;		///< ...unless they are any of these
	KindOfMaskType highValueKinds;		///< map buildings that make a point high-value and follow its owner
	KindOfMaskType defenseKinds;		///< structures the neutral-neighbor build rule allows
	KindOfMaskType aiBuildListRemovals;	///< taken out of the AI's build lists: there are no supplies to gather

	Real aiThinkSeconds = 3.0f;			///< how often the AI picks sectors to go for
	Int aiGroupSize = 3;				///< units the AI sends to each target
	Int aiMaxTargets = 4;				///< targets the AI works on at once
	std::vector<ScriptRename> aiScriptRenames;	///< tried in order; each AIScriptRename line adds one

	AsciiString pointObject;			///< the object that marks and stores each sector point

	Config();
};

const Config &config();
void parseConfigBlock(INI *ini);
void resetConfig();

UnsignedInt secondsToFrames(Real seconds);

} // namespace CoH::Sectors
