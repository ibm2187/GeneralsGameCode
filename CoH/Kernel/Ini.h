// CoH Kernel: typed INI fields, for C++20 feature code. A table names each field once, with a member
// pointer instead of offsetof, and the parser comes from the member's type:
//
//     void parseConfigBlock(INI *ini)	// registered with CoH::registerIniBlock("CoHCover", ...)
//     {
//         static const CoH::Ini::Table<Config> fields = {
//             CoH::Ini::field("Radius", &Config::radius),							// Real
//             CoH::Ini::field("CoverKinds", &Config::coverKinds),					// KindOfMaskType
//             CoH::Ini::field("HeavyBonus", &Config::rules, &CoverRules::heavyBonus),	// inside a Domain struct
//             CoH::Ini::seconds("Delay", &Config::delayFrames),					// seconds in, frames kept
//             CoH::Ini::percent("MissChance", &Config::missChance),				// "25%" is 0.25
//         };
//         fields.read(ini, storage());
//     }
//
// A module passes its table to MultiIniFieldParse::add in buildFieldParse: p.add(fields).

#pragma once

#include "Common/INI.h"
#include "Common/KindOf.h"

#include <concepts>
#include <cstddef>
#include <initializer_list>
#include <vector>

namespace CoH::Ini
{

// The engine parser for each member type a field can have. Another type doesn't compile in field();
// read it with custom().
template <class T> struct Parser { };
template <> struct Parser<Bool> { static constexpr INIFieldParseProc parse = &INI::parseBool; };
template <> struct Parser<Int> { static constexpr INIFieldParseProc parse = &INI::parseInt; };
template <> struct Parser<UnsignedInt> { static constexpr INIFieldParseProc parse = &INI::parseUnsignedInt; };
template <> struct Parser<Real> { static constexpr INIFieldParseProc parse = &INI::parseReal; };
template <> struct Parser<AsciiString> { static constexpr INIFieldParseProc parse = &INI::parseAsciiString; };
template <> struct Parser<KindOfMaskType> { static constexpr INIFieldParseProc parse = &KindOfMaskType::parseFromINI; };

template <class T>
concept Parsable = requires { { Parser<T>::parse } -> std::convertible_to<INIFieldParseProc>; };

// Reads "Delay = 2.5" (seconds) into whole logic frames, rounded up.
void parseSecondsAsFrames(INI *ini, void *instance, void *store, const void *userData);

namespace Detail
{

// Where a member sits in its class, from a member pointer: offsetof without naming the member twice.
// Only the address is taken; nothing is read or constructed.
template <class Owner, class T>
Int offsetOf(T Owner::*member)
{
	alignas(Owner) static std::byte storage[sizeof(Owner)];
	const Owner *owner = reinterpret_cast<const Owner *>(storage);
	return static_cast<Int>(reinterpret_cast<const std::byte *>(&(owner->*member)) - storage);
}

} // namespace Detail

// One line of a table, for an INI block that fills an Owner.
template <class Owner>
struct Field
{
	FieldParse parse;
};

template <class Owner, Parsable T>
Field<Owner> field(const char *token, T Owner::*member)
{
	return { { token, Parser<T>::parse, nullptr, Detail::offsetOf(member) } };
}

// A member of a struct inside Owner, such as Domain rules inside a Config.
template <class Owner, class Inner, Parsable T>
Field<Owner> field(const char *token, Inner Owner::*outer, T Inner::*member)
{
	return { { token, Parser<T>::parse, nullptr, Detail::offsetOf(outer) + Detail::offsetOf(member) } };
}

template <class Owner>
Field<Owner> seconds(const char *token, UnsignedInt Owner::*frames)
{
	return { { token, &parseSecondsAsFrames, nullptr, Detail::offsetOf(frames) } };
}

template <class Owner, class Inner>
Field<Owner> seconds(const char *token, Inner Owner::*outer, UnsignedInt Inner::*frames)
{
	return { { token, &parseSecondsAsFrames, nullptr, Detail::offsetOf(outer) + Detail::offsetOf(frames) } };
}

template <class Owner>
Field<Owner> percent(const char *token, Real Owner::*fraction)
{
	return { { token, &INI::parsePercentToReal, nullptr, Detail::offsetOf(fraction) } };
}

template <class Owner, class Inner>
Field<Owner> percent(const char *token, Inner Owner::*outer, Real Inner::*fraction)
{
	return { { token, &INI::parsePercentToReal, nullptr, Detail::offsetOf(outer) + Detail::offsetOf(fraction) } };
}

// A field the parser handles itself, given the whole Owner as 'instance' (and as 'store'): lists,
// pairs, lookups.
template <class Owner>
Field<Owner> custom(const char *token, INIFieldParseProc parse, const void *userData = nullptr)
{
	return { { token, parse, userData, 0 } };
}

// The engine's field table, ended for it. Keep it static: the engine holds on to the pointer.
template <class Owner>
class Table
{
public:
	Table(std::initializer_list<Field<Owner>> fields)
	{
		m_fields.reserve(fields.size() + 1);
		for (const Field<Owner> &field : fields)
			m_fields.push_back(field.parse);
		m_fields.push_back({ nullptr, nullptr, nullptr, 0 });
	}

	operator const FieldParse *() const { return m_fields.data(); }

	// Reads the rest of the block into 'into', as INI::initFromINI.
	void read(INI *ini, Owner &into) const { ini->initFromINI(&into, m_fields.data()); }

private:
	std::vector<FieldParse> m_fields;
};

} // namespace CoH::Ini
