// CoH VehicleMovement: how a vehicle's weight changes its handling, as pure code. Unit tested in
// Tests/HandlingTest.cpp.
//
// Each handling value has a curve of weight (tons) to a factor on the retail value. The pack's curves
// put 100% at the Medium class, so a vehicle of that weight drives as in retail.

#pragma once

#include <initializer_list>
#include <vector>

namespace CoH::VehicleMovement
{

struct CurvePoint
{
	float weight = 0.0f;	///< tons
	float value = 1.0f;		///< the factor at that weight
};

// Straight lines between the points, flat beyond the first and the last. Plain arithmetic only, so it
// gives the same result on every machine.
class Curve
{
public:
	Curve() = default;
	Curve(std::initializer_list<CurvePoint> points);

	void add(CurvePoint point);	///< kept in weight order; a point at a weight already there replaces it
	void clear() { m_points.clear(); }
	bool empty() const { return m_points.empty(); }

	float at(float weight, float whenEmpty) const;

private:
	std::vector<CurvePoint> m_points;
};

struct HandlingCurves
{
	Curve speed;			///< top speed
	Curve acceleration;
	Curve braking;
	Curve turnRate;
	Curve slopePenalty;		///< the share of speed lost going up a 100% grade
	float slopeMinSpeed = 0.4f;	///< a slope never slows below this share of the speed
};

// The factors for one weight. Every factor is at least MIN_FACTOR, so nothing stops or divides by 0.
struct Handling
{
	float speed = 1.0f;
	float acceleration = 1.0f;
	float braking = 1.0f;
	float turnRate = 1.0f;
	float slopePenalty = 0.0f;
};

inline constexpr float MIN_FACTOR = 0.05f;

Handling handlingFor(float weight, const HandlingCurves &curves);

// The share of its speed a vehicle keeps on ground that rises 'grade' (height over distance) ahead of
// it. Flat ground and downhill keep it all.
float slopeSpeedFactor(float grade, float penalty, float minSpeed);

// A vehicle's weight: its own and what its upgrades add. Never below 0.
float totalWeight(float base, std::initializer_list<float> added);

} // namespace CoH::VehicleMovement
