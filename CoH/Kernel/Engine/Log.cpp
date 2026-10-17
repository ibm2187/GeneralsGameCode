// CoH Kernel: see Kernel/Log.h.

#include "PreRTS.h"	// This must go first in EVERY cpp file in the GameEngine

#include "Common/GlobalData.h"

#include "Kernel/Domain/Arguments.h"
#include "Kernel/Engine/Internal.h"
#include "Kernel/Log.h"

#include <cstdarg>
#include <cstdio>

namespace CoH
{

namespace
{

FILE *openLog()
{
	AsciiString path = TheGlobalData ? TheGlobalData->getPath_UserData() : AsciiString::TheEmptyString;
	path.concat("CoH.log");
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
