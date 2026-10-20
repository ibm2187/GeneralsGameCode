// CoH VehicleMovement: see Handling.h.

#include "Features/VehicleMovement/Domain/Handling.h"

#include <algorithm>

namespace CoH::VehicleMovement
{

Curve::Curve(std::initializer_list<CurvePoint> points)
{
	for (const CurvePoint &point : points)
		add(point);
}

void Curve::add(CurvePoint point)
{
	const auto place = std::ranges::lower_bound(m_points, point.weight, {}, &CurvePoint::weight);
	if (place != m_points.end() && place->weight == point.weight)
		*place = point;
	else
		m_points.insert(place, point);
}

float Curve::at(float weight, float whenEmpty) const
{
	if (m_points.empty())
		return whenEmpty;
	if (weight <= m_points.front().weight)
		return m_points.front().value;
	if (weight >= m_points.back().weight)
		return m_points.back().value;
	const auto above = std::ranges::upper_bound(m_points, weight, {}, &CurvePoint::weight);
	const CurvePoint &high = *above;
	const CurvePoint &low = *(above - 1);
	const float share = (weight - low.weight) / (high.weight - low.weight);
	return low.value + (high.value - low.value) * share;
}

namespace
{

float factor(const Curve &curve, float weight)
{
	return std::max(curve.at(weight, 1.0f), MIN_FACTOR);
}

} // namespace

Handling handlingFor(float weight, const HandlingCurves &curves)
{
	return {
		.speed = factor(curves.speed, weight),
		.acceleration = factor(curves.acceleration, weight),
		.braking = factor(curves.braking, weight),
		.turnRate = factor(curves.turnRate, weight),
		.slopePenalty = std::max(curves.slopePenalty.at(weight, 0.0f), 0.0f),
	};
}

float slopeSpeedFactor(float grade, float penalty, float minSpeed)
{
	if (grade <= 0.0f || penalty <= 0.0f)
		return 1.0f;
	const float floor = std::clamp(minSpeed, MIN_FACTOR, 1.0f);
	return std::clamp(1.0f - grade * penalty, floor, 1.0f);
}

float totalWeight(float base, std::initializer_list<float> added)
{
	float weight = base;
	for (float tons : added)
		weight += tons;
	return std::max(weight, 0.0f);
}

} // namespace CoH::VehicleMovement
