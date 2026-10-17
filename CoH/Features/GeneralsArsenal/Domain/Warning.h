// CoH GeneralsArsenal: the super weapon warning. Super weapons need no building, so the counterplay
// is time: after the target is picked, every player sees it marked with a countdown, and only then
// does the weapon fire. Times are in logic frames.

#pragma once

#include <string>

namespace CoH::GeneralsArsenal
{

class WarningTimer
{
public:
	WarningTimer() = default;
	WarningTimer(unsigned int startFrame, unsigned int delayFrames) : m_start(startFrame), m_delay(delayFrames) {}

	unsigned int fireFrame() const { return m_start + m_delay; }
	bool isDue(unsigned int now) const { return now >= fireFrame(); }

	// Whole seconds left, rounded up: the count reads 3, 2, 1 and fires when it would read 0.
	int secondsLeft(unsigned int now, int framesPerSecond) const;

	// How far along the warning is: 0 at the start, 1 when it fires.
	float progress(unsigned int now) const;

	unsigned int startFrame() const { return m_start; }
	unsigned int delayFrames() const { return m_delay; }

private:
	unsigned int m_start = 0;
	unsigned int m_delay = 0;
};

// "1:05", "0:09".
std::string formatCountdown(int seconds);

} // namespace CoH::GeneralsArsenal
