// CoH Kernel: game time as pure code. Game logic counts time in logic frames, 30 to the second on
// every machine; INI tuning is written in seconds. Kernel/Time.h adds the current frame.

#pragma once

namespace CoH
{

inline constexpr unsigned int FramesPerSecond = 30;

// Seconds as whole frames, rounded up so a short time never becomes zero frames. Float noise below
// a thousandth of a frame doesn't round up (0.1 s is 3 frames, not 4). Zero or less is 0 frames.
constexpr unsigned int framesFromSeconds(float seconds)
{
	if (!(seconds > 0.0f))
		return 0;
	const float exact = seconds * FramesPerSecond;
	if (exact >= 4294967040.0f)	// the largest float below 2^32
		return 4294967295u;
	const auto whole = static_cast<unsigned int>(exact);
	return exact - static_cast<float>(whole) > 0.001f ? whole + 1 : whole;
}

constexpr float secondsFromFrames(unsigned int frames)
{
	return static_cast<float>(frames) / FramesPerSecond;
}

// Whether 'frame' starts a new period of 'period' frames. 'offset' shifts the periods, to spread
// work out (one player per frame). Frame 0 starts one; a period of 0 never does.
constexpr bool isEvery(unsigned int frame, unsigned int period, unsigned int offset = 0)
{
	return period > 0 && (frame + offset) % period == 0;
}

} // namespace CoH
