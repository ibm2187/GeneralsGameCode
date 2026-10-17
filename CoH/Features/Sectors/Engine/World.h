// CoH Sectors: the sectors of the game being played. The point objects on the map (SectorPointUpdate)
// hold the saved state; the World is made from them on the first frame of a game, new or loaded, and
// keeps what is worked out from them: the sector map, who is inside each point, and each player's
// connected sectors and income.

#pragma once

#include "Features/Sectors/Domain/Capture.h"
#include "Features/Sectors/Domain/SectorMap.h"

#include <vector>

class Object;
class Player;
class SectorPointUpdate;
struct Coord3D;

namespace CoH::Sectors
{

struct Sector
{
	ObjectID point = INVALID_ID;
	Vec2 position;
	SectorKind kind = SectorKind::Supply;
	PlayerId homePlayer = NoPlayer;
};

class World
{
public:
	void clear();

	// Made from the point objects; false while there are none, as in a game without sectors.
	bool build();
	bool isBuilt() const { return m_built; }
	int buildNumber() const { return m_buildNumber; }	///< new for every build, so the UI knows to redo its drawing
	bool hasSectors() const { return !m_sectors.empty(); }

	int sectorCount() const { return (int)m_sectors.size(); }
	const Sector &sector(int index) const { return m_sectors[index]; }
	const SectorMap &map() const { return m_map; }

	// The point module of a sector; null if its object is gone.
	SectorPointUpdate *point(int index) const;
	PlayerId ownerOf(int index) const;
	std::vector<PlayerId> owners() const;
	const std::vector<PlayerId> &ownersAtRefresh() const { return m_owners; }	///< as of the last refreshNetwork
	int sectorAt(const Coord3D &position) const;

	// Who was inside each point at the last capture step.
	void setOccupation(int index, const Occupation &occupation) { m_occupation[index] = occupation; }
	const Occupation &occupation(int index) const { return m_occupation[index]; }

	// Recomputes every player's connected sectors and income from the current owners.
	void refreshNetwork();
	bool isConnected(PlayerId player, int index) const;
	bool isCutOff(int index) const;	///< held, but not joined to the owner's roots
	const std::vector<bool> &connectedOf(PlayerId player) const;
	int incomePerPeriod(PlayerId player) const;
	int incomePerMinute(PlayerId player) const;

	// The sectors a player's supply lines start from: the home sector, and sectors with one of the
	// player's command centers.
	std::vector<int> rootsOf(PlayerId player) const;

	// A home whose owner still has a command center can't be captured.
	bool isLocked(int index) const;

private:
	void findCommandCenters();

	bool m_built = false;
	int m_buildNumber = 0;
	std::vector<Sector> m_sectors;
	SectorMap m_map;
	std::vector<Occupation> m_occupation;
	std::vector<PlayerId> m_owners;
	std::vector<std::vector<bool>> m_connected;	///< per player index
	std::vector<int> m_income;					///< per player index, one period's pay
	std::vector<bool> m_hasCommandCenter;		///< per player index
	std::vector<std::vector<int>> m_commandCenterSectors;	///< per player index
};

World &world();

// The engine's answers to Domain questions.
bool areEnemies(PlayerId a, PlayerId b);
Player *findPlayer(PlayerId player);
Vec2 toVec2(const Coord3D &position);

} // namespace CoH::Sectors
