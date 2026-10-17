// CoH Sectors: see Points.h.

#include "Points.h"

#include <numeric>

namespace CoH::Sectors
{

namespace
{

// Groups of source indices, each a chain of sources no farther than the merge distance apart.
std::vector<std::vector<int>> clusterSources(const std::vector<Source> &sources, float mergeDistance)
{
	std::vector<int> root(sources.size());
	std::iota(root.begin(), root.end(), 0);
	const auto find = [&root](int i)
	{
		while (root[i] != i)
			i = root[i] = root[root[i]];
		return i;
	};

	const float limit = mergeDistance * mergeDistance;
	for (size_t a = 0; a < sources.size(); ++a)
	{
		for (size_t b = a + 1; b < sources.size(); ++b)
		{
			if (distanceSquared(sources[a].position, sources[b].position) <= limit)
				root[find((int)b)] = find((int)a);
		}
	}

	// In the order of each cluster's first source, so the plan doesn't depend on anything else.
	std::vector<std::vector<int>> clusters;
	std::vector<int> clusterOfRoot(sources.size(), -1);
	for (size_t i = 0; i < sources.size(); ++i)
	{
		const int r = find((int)i);
		if (clusterOfRoot[r] < 0)
		{
			clusterOfRoot[r] = (int)clusters.size();
			clusters.emplace_back();
		}
		clusters[clusterOfRoot[r]].push_back((int)i);
	}
	return clusters;
}

PointPlan pointFromCluster(const std::vector<Source> &sources, const std::vector<int> &cluster)
{
	PointPlan point;
	for (int i : cluster)
	{
		point.position.x += sources[i].position.x;
		point.position.y += sources[i].position.y;
		point.members.push_back(sources[i].id);
		if (sources[i].kind == SectorKind::HighValue)
			point.kind = SectorKind::HighValue;
	}
	point.position.x /= (float)cluster.size();
	point.position.y /= (float)cluster.size();
	return point;
}

// The start whose home a cluster joins: the nearest one within reach of any of its sources.
int findHomeFor(const std::vector<Source> &sources, const std::vector<int> &cluster, const std::vector<Start> &starts,
	float homeMergeDistance)
{
	int best = -1;
	float bestDistance = homeMergeDistance * homeMergeDistance;
	for (size_t s = 0; s < starts.size(); ++s)
	{
		for (int i : cluster)
		{
			const float d = distanceSquared(sources[i].position, starts[s].position);
			if (d <= bestDistance)
			{
				best = (int)s;
				bestDistance = d;
			}
		}
	}
	return best;
}

} // namespace

std::vector<PointPlan> planPoints(const std::vector<Source> &sources, const std::vector<Start> &starts,
	const PointRules &rules)
{
	std::vector<PointPlan> points;
	for (const Start &start : starts)
	{
		PointPlan home;
		home.position = start.position;
		home.kind = SectorKind::Home;
		home.homePlayer = start.player;
		points.push_back(home);
	}

	for (const std::vector<int> &cluster : clusterSources(sources, rules.mergeDistance))
	{
		const int home = findHomeFor(sources, cluster, starts, rules.homeMergeDistance);
		if (home < 0)
		{
			points.push_back(pointFromCluster(sources, cluster));
			continue;
		}

		for (int i : cluster)
			points[home].members.push_back(sources[i].id);
	}
	return points;
}

} // namespace CoH::Sectors
