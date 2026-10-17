# 01 Sectors

**Branch** `coh/sectors` · **Needs** 00 · **Slice** `CoH/Features/Sectors/` · **Flag** `Sectors`

## Goal

Supplies come from territory. The map is split into sectors around capture points, and owning sectors is the economy.
There are no supply centers, supply trucks, supply docks, supply stashes, drop zones or gathering.

## Behavior

- **Sectors from any map:** on game start, every supply source (docks, warehouses, supply piles) becomes a sector point.
  Oil derricks and other tech buildings become high-value points.
  Each point owns the ground nearest to it, so the whole map is divided.
  Sectors next to each other are neighbors.
- **Home sector:** each player owns the sector around their start position from the first frame. It can't be captured while their command center stands.
- **Capture by conquest:**
  - Units standing within a point's radius push its capture toward their side.
  - Any combat unit counts; which kinds count is set in INI (infantry, vehicles, helicopters).
  - If units of two sides are inside, the point is contested and nothing moves.
  - An enemy point is first neutralized, then captured.
  - There is no capture upgrade and no capture-building ability.
  - Speed scales with the number of capturing units, up to a cap.
- **Tech buildings:** a captured derrick or tech building belongs to whoever holds its sector, through the same conquest rule.
- **Income:** every few seconds a player earns money for each sector they hold, with a value per sector type.
  - **Cut off:** a sector only pays if a chain of the player's own sectors joins it to their home sector. A cut-off sector shows as cut off.
- **Base building:** players may build only in sectors they own. Neutral ground near the home sector allows defenses; the exact rule is set in INI.
- **Removed from build lists:** supply centers, supply trucks, the Chinook's supply role, supply stashes and drop zones.
  The GLA Worker and China Dozer stay as builders, but don't gather.
- **AI:**
  - Skirmish AI sends groups to capture and hold sectors, starting with the nearest and the cut-off-critical ones.
  - It retakes lost sectors and defends contested ones.
  - It no longer builds gatherers.

## UI

- Sector borders and owner color on the ground and the minimap.
- A capture-progress ring at each point. Contested and cut-off states are shown.
- Income per minute in the money tooltip.

## Likely hook points

Game start after the map loads (to build sectors); logic frame (capture and income); can-build-at-location; minimap draw; the command set and build list come from the pack.

## Done

- Domain tests cover the capture progress math, contesting, neutralizing, the cut-off graph walk and income.
- A full skirmish against the AI on two retail maps is playable, and the AI expands and fights over points.
- With the flag off, supply play is unchanged.
