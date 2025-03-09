#include "pch.h"
#include <optional>
#include <algorithm>

namespace egkr::parser
{
    static std::optional<std::pair<std::string, std::string>> parse_line(const std::string& line, char separator = '=', const std::string& continue_chars = "#\n")
    {
	std::string l = line;
	trim(l);

	if (l.empty() || l == "" || l[0] == '\0')
	{
	    return {};
	}

	if (std::ranges::any_of(continue_chars, [&](char continue_char) { return l[0] == continue_char; }))
	{
	    return {};
	}

	const auto split_index = l.find_first_of(separator);
	if (split_index == std::string::npos)
	{
	    LOG_ERROR("Potential formatting issue, {}", line);
	    return {{line, ""}};
	}

	auto variable_name = l.substr(0, split_index);
	trim(variable_name);

	auto value = l.substr(split_index + 1);
	trim(value);

	return {{variable_name, value}};
    }
}
