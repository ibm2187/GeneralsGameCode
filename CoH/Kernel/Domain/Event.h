// CoH Kernel: one hook point features subscribe to. Raising it calls only the subscribers whose
// feature is on for this game, so a feature that is off never runs: off means original.

#pragma once

#include "Kernel/Domain/Feature.h"

#include <vector>

namespace CoH
{

template <typename... Args>
class Event
{
public:
	using Handler = void (*)(Args...);

	void subscribe(const Feature &owner, Handler handler)
	{
		if (handler)
			m_subscribers.push_back({ &owner, handler });
	}

	void raise(Args... args) const
	{
		for (const Subscriber &subscriber : m_subscribers)
		{
			if (subscriber.owner->isActive())
				subscriber.handler(args...);
		}
	}

private:
	struct Subscriber
	{
		const Feature *owner;
		Handler handler;
	};

	std::vector<Subscriber> m_subscribers;
};

} // namespace CoH
