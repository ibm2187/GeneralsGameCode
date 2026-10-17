// CoH Kernel: the local flags. "-coh Sectors,Accuracy" on the command line sets them for this run
// ("-coh none" turns every feature off); otherwise they come from Features.ini in the user data folder.

#include "PreRTS.h"	// This must go first in EVERY cpp file in the GameEngine

#include "Common/GlobalData.h"
#include "Common/LocalFileSystem.h"
#include "Common/file.h"

#include "Kernel/Domain/Arguments.h"
#include "Kernel/Engine/Internal.h"

#include <stdlib.h>

namespace CoH::Kernel
{

namespace
{

std::string readUserDataFile(const char *fileName)
{
	AsciiString path = TheGlobalData->getPath_UserData();
	path.concat(fileName);

	File *file = TheLocalFileSystem->openFile(path.str(), File::READ);
	if (file == nullptr)
		return std::string();

	const Int size = file->size();
	char *buffer = file->readEntireAndClose();
	std::string text(buffer, size);
	delete[] buffer;
	return text;
}

} // namespace

std::vector<std::string> processArguments()
{
	std::vector<std::string> arguments;
	for (Int i = 1; i < __argc; ++i)
		arguments.push_back(__argv[i]);
	return arguments;
}

FeatureSet readLocalFlags()
{
	if (const std::optional<std::string> list = findOption(processArguments(), "-coh"))
		return FeatureSet::fromList(*list);

	return FeatureSet::fromSettingsFile(readUserDataFile("Features.ini"));
}

} // namespace CoH::Kernel
