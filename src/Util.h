#pragma once

namespace DDR::Util
{
	inline std::vector<std::string> Split(const std::string& a_str, std::string_view a_delimiter)
	{
		auto range = a_str | std::ranges::views::split(a_delimiter) | std::ranges::views::transform([](auto&& r) { return std::string_view(r); });
		return { range.begin(), range.end() };
	}

	inline std::string Join(const std::vector<std::string>& a_vec, std::string_view a_delimiter)
	{
		return std::accumulate(a_vec.begin(), a_vec.end(), std::string{},
			[a_delimiter](const auto& str1, const auto& str2) {
				return str1.empty() ? str2 : str1 + a_delimiter.data() + str2;
			});
	}
}