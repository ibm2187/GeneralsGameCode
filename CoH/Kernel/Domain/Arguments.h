// CoH Kernel: the Kernel's own command-line options. The engine skips options it doesn't know,
// so the Kernel reads these straight from the process arguments without a hook.
// Plain C++ with no engine headers, so it is unit tested.

#pragma once

#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace CoH
{

// The value after an option ("-coh Sectors" gives "Sectors"), an empty value when the option is
// last or followed by another option, and nothing when the option isn't there. Options match
// without case, like the engine's own. The last one wins when an option is given twice.
std::optional<std::string> findOption(const std::vector<std::string> &arguments, std::string_view option);

} // namespace CoH
