// CoH Kernel: small string helpers for the Domain code, which has no engine strings.

#pragma once

#include <string_view>
#include <vector>

namespace CoH::Text
{

std::string_view trim(std::string_view text);

// Pieces between separators, trimmed, without empty pieces.
std::vector<std::string_view> split(std::string_view text, char separator);

bool equalsNoCase(std::string_view a, std::string_view b);
bool lessNoCase(std::string_view a, std::string_view b);

} // namespace CoH::Text
