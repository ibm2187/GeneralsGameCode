// CoH Kernel: see Arguments.h.

#include "Kernel/Domain/Arguments.h"
#include "Kernel/Domain/Text.h"

namespace CoH
{

std::optional<std::string> findOption(const std::vector<std::string> &arguments, std::string_view option)
{
	std::optional<std::string> value;
	for (size_t i = 0; i < arguments.size(); ++i)
	{
		if (!Text::equalsNoCase(arguments[i], option))
			continue;

		const bool hasValue = i + 1 < arguments.size() && arguments[i + 1].rfind('-', 0) != 0;
		value = hasValue ? arguments[i + 1] : std::string();
	}
	return value;
}

} // namespace CoH
