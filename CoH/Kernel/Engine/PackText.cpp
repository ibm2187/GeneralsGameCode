// CoH Kernel: text a pack adds. A CoHText block lists labels and their text:
//
//     CoHText
//       Text = CONTROLBAR:CoHRaptorStrike "Raptor Strike"
//       Text = CONTROLBAR:TooltipCoHRaptorStrike "Raptors fly in, strike the target and leave."
//     End
//
// The game's string file is asked first; only a label it lacks is looked up here (GameTextManager::fetch).
// "\n" in the text starts a new line.

#include "PreRTS.h"	// This must go first in EVERY cpp file in the GameEngine

#include "Common/INI.h"

#include "Kernel/Engine/Internal.h"

#include <map>
#include <string>

namespace CoH::PackText
{

namespace
{

std::map<std::string, UnicodeString> &texts()
{
	static std::map<std::string, UnicodeString> theTexts;
	return theTexts;
}

UnicodeString toText(const AsciiString &line)
{
	const char backslash = 92;
	AsciiString unescaped;
	for (const char *c = line.str(); *c; ++c)
	{
		if (c[0] == backslash && c[1] == 'n')
		{
			unescaped.concat('\n');
			++c;
		}
		else
		{
			unescaped.concat(*c);
		}
	}
	UnicodeString text;
	text.translate(unescaped);
	return text;
}

void parseText(INI *ini, void * /*instance*/, void * /*store*/, const void * /*userData*/)
{
	const std::string label = ini->getNextToken();
	texts()[label] = toText(ini->getNextQuotedAsciiString());
}

const FieldParse textFields[] =
{
	{ "Text", parseText, nullptr, 0 },
	{ nullptr, nullptr, nullptr, 0 }
};

} // namespace

void parseBlock(INI *ini)
{
	ini->initFromINI(&texts(), textFields);
}

const UnicodeString *find(const char *label)
{
	if (label == nullptr || texts().empty())
		return nullptr;
	const auto it = texts().find(label);
	return it != texts().end() ? &it->second : nullptr;
}

void clear()
{
	texts().clear();
}

} // namespace CoH::PackText
