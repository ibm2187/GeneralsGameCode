// CoH Kernel: see Text.h.

#include "Kernel/Domain/Text.h"

#include <algorithm>

namespace CoH::Text
{

namespace
{

bool isSpace(char c)
{
	return c == ' ' || c == '\t' || c == '\r' || c == '\n';
}

char lower(char c)
{
	return (c >= 'A' && c <= 'Z') ? static_cast<char>(c - 'A' + 'a') : c;
}

} // namespace

std::string_view trim(std::string_view text)
{
	while (!text.empty() && isSpace(text.front()))
		text.remove_prefix(1);
	while (!text.empty() && isSpace(text.back()))
		text.remove_suffix(1);
	return text;
}

std::vector<std::string_view> split(std::string_view text, char separator)
{
	std::vector<std::string_view> pieces;
	while (!text.empty())
	{
		const size_t end = text.find(separator);
		const std::string_view piece = trim(text.substr(0, end));
		if (!piece.empty())
			pieces.push_back(piece);
		if (end == std::string_view::npos)
			break;
		text.remove_prefix(end + 1);
	}
	return pieces;
}

bool equalsNoCase(std::string_view a, std::string_view b)
{
	return std::equal(a.begin(), a.end(), b.begin(), b.end(),
		[](char x, char y) { return lower(x) == lower(y); });
}

bool lessNoCase(std::string_view a, std::string_view b)
{
	return std::lexicographical_compare(a.begin(), a.end(), b.begin(), b.end(),
		[](char x, char y) { return lower(x) < lower(y); });
}

} // namespace CoH::Text
