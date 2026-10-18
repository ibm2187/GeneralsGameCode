// CoH Kernel: see Kernel/Log.h.

#include "PreRTS.h"	// This must go first in EVERY cpp file in the GameEngine

#include "Common/GlobalData.h"
#include "GameClient/ClientInstance.h"

#include "Kernel/Domain/Arguments.h"
#include "Kernel/Engine/Internal.h"
#include "Kernel/Log.h"

#include <cstdarg>
#include <cstdio>

namespace CoH
{

namespace
{

// CoH.log; a second game running at the same time writes CoH_Instance02.log, as its replay is named.
// "-cohLog <name>" names the file instead, for test games run side by side.
FILE *openLog()
{
	AsciiString path = TheGlobalData ? TheGlobalData->getPath_UserData() : AsciiString::TheEmptyString;
	const std::optional<std::string> name = findOption(Kernel::processArguments(), "-cohLog");
	if (name && !name->empty())
	{
		path.concat(name->c_str());
	}
	else if (rts::ClientInstance::getInstanceId() > 1u)
	{
		AsciiString instanceName;
		instanceName.format("CoH_Instance%.2u.log", rts::ClientInstance::getInstanceId());
		path.concat(instanceName);
	}
	else
	{
		path.concat("CoH.log");
	}
	return fopen(path.str(), "w");
}

FILE *logFile()
{
	static FILE *file = isLogging() ? openLog() : nullptr;
	return file;
}

} // namespace

bool isLogging()
{
	static const bool logging = findOption(Kernel::processArguments(), "-cohLog").has_value();
	return logging;
}

void log(const char *format, ...)
{
	char line[1024];
	va_list args;
	va_start(args, format);
	vsnprintf(line, sizeof(line), format, args);
	va_end(args);

	DEBUG_LOG(("CoH: %s", line));

	FILE *file = logFile();
	if (file == nullptr)
		return;
	fprintf(file, "%s\n", line);
	fflush(file);
}

} // namespace CoH
