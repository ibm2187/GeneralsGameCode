// CoH GeneralsArsenal: see Warning.h.

#include "Warning.h"

#include <cstdio>

namespace CoH::GeneralsArsenal
{

int WarningTimer::secondsLeft(unsigned int now, int framesPerSecond) const
{
	if (isDue(now) || framesPerSecond <= 0)
		return 0;
	const unsigned int left = fireFrame() - (now > m_start ? now : m_start);
	return (int)((left + framesPerSecond - 1) / framesPerSecond);
}

float WarningTimer::progress(unsigned int now) const
{
	if (m_delay == 0 || isDue(now))
		return 1.0f;
	if (now <= m_start)
		return 0.0f;
	return (float)(now - m_start) / (float)m_delay;
}

std::string formatCountdown(int seconds)
{
	if (seconds < 0)
		seconds = 0;
	char text[16];
	std::snprintf(text, sizeof(text), "%d:%02d", seconds / 60, seconds % 60);
	return text;
}

} // namespace CoH::GeneralsArsenal
