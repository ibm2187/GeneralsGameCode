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

	// A feature that is on listens, so the original code may hand the job to the hub.
	bool hasActiveSubscriber() const
	{
		for (const Subscriber &subscriber : m_subscribers)
		{
			if (subscriber.owner->isActive())
				return true;
		}
		return false;
	}

private:
	struct Subscriber
	{
		const Feature *owner;
		Handler handler;
	};

	std::vector<Subscriber> m_subscribers;
};

// A question original code asks, such as "may this be built here?". The answer is yes unless a
// subscriber whose feature is on says no, so with every feature off the original answer stands.
template <typename... Args>
class Veto
{
public:
	using Handler = bool (*)(Args...);	///< false says no

	void subscribe(const Feature &owner, Handler handler)
	{
		if (handler)
			m_subscribers.push_back({ &owner, handler });
	}

	bool allows(Args... args) const
	{
		for (const Subscriber &subscriber : m_subscribers)
		{
			if (subscriber.owner->isActive() && !subscriber.handler(args...))
				return false;
		}
		return true;
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
