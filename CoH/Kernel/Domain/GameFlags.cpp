// CoH Kernel: see GameFlags.h.

#include "Kernel/Domain/GameFlags.h"
#include "Kernel/Domain/Text.h"

namespace CoH
{

FeatureSet chooseGameFlags(GameOrigin origin, const FeatureSet &local, const FeatureSet &fromGameOptions,
	const FeatureSet &fromSave)
{
	switch (origin)
	{
	case GameOrigin::ShellMap: return FeatureSet();
	case GameOrigin::LocalGame: return local;
	case GameOrigin::RemoteGame: return fromGameOptions;
	case GameOrigin::Replay: return fromGameOptions;
	case GameOrigin::SaveGame: return fromSave;
	}
	return FeatureSet();
}

std::string formatGameOption(const FeatureSet &flags)
{
	if (flags.empty())
		return std::string();

	return std::string(GameOptionKey) + '=' + flags.toList() + ';';
}

FeatureSet findGameOption(std::string_view options)
{
	for (std::string_view option : Text::split(options, ';'))
	{
		const size_t equals = option.find('=');
		if (equals == std::string_view::npos)
			continue;

		if (Text::trim(option.substr(0, equals)) == GameOptionKey)
			return FeatureSet::fromList(option.substr(equals + 1));
	}
	return FeatureSet();
}

} // namespace CoH
