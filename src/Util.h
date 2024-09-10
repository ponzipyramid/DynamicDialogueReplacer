#pragma once

namespace DDR::Util
{
	using SKSE::stl::enumeration;
	using SKSE::stl::report_and_fail;
	using SKSE::stl::to_underlying;

	const char* ws = " \t\n\r\f\v";

	inline std::vector<std::string> Split(const std::string& a_str, std::string_view a_delimiter)
	{
		auto range = a_str | std::ranges::views::split(a_delimiter) | std::ranges::views::transform([](auto&& r) { return std::string_view(r); });
		return { range.begin(), range.end() };
	}

	inline int GetRandInt(int a_min, int a_max)
	{
		std::random_device rand_dev;
		std::mt19937 generator(rand_dev());
		std::uniform_int_distribution<int> distr(a_min, a_max);
		return distr(generator);
	}

	inline std::string Join(const std::vector<std::string>& a_vec, std::string_view a_delimiter)
	{
		return std::accumulate(a_vec.begin(), a_vec.end(), std::string{},
			[a_delimiter](const auto& str1, const auto& str2) {
				return str1.empty() ? str2 : str1 + a_delimiter.data() + str2;
			});
	}

	inline std::string str_toupper(std::string s)
	{
		std::transform(
			s.begin(),
			s.end(),
			s.begin(),
			[](unsigned char c) {
				return static_cast<char>(std::toupper(c));
			});
		return s;
	}

	inline std::string& rtrim(std::string& s, const char* t = ws)
	{
		s.erase(s.find_last_not_of(t) + 1);
		return s;
	}

	inline std::string& ltrim(std::string& s, const char* t = ws)
	{
		s.erase(0, s.find_first_not_of(t));
		return s;
	}

	inline std::string& trim(std::string& s, const char* t = ws)
	{
		return ltrim(rtrim(s, t), t);
	}

	inline std::optional<std::pair<RE::FormID, std::string>> ParseFormId(std::string a_str)
	{
		const auto splits = Split(a_str, "|"sv);

		if (splits.size() != 2) {
			return std::nullopt;
		}

		char* p;
		const auto formId = std::strtol(splits[0].c_str(), &p, 16);

		if (*p != 0) {
			return std::nullopt;
		}

		const auto espName = std::regex_replace(splits[1], std::regex("^ +| +$|( ) +"), "$1");

		return std::make_pair(formId, espName);
	}

	template <typename T = RE::TESForm>
	inline T* GetFormFromString(const std::string& s)
	{
		if (auto form = RE::TESForm::LookupByEditorID(s)) {
			return form->As<T>();
		}

		if (const auto parsed = ParseFormId(s)) {
			const auto [formId, espName] = parsed.value();
			return RE::TESDataHandler::GetSingleton()->LookupForm<T>(formId, espName);
		} else {
			return nullptr;
		}
	}

	inline std::string GenerateUUID()
	{
		static std::random_device dev;
		static std::mt19937 rng(dev());

		std::uniform_int_distribution<int> dist(0, 15);

		const char* v = "0123456789abcdef";
		const bool dash[] = { 0, 0, 0, 0, 1, 0, 1, 0, 1, 0, 1, 0, 0, 0, 0, 0 };

		std::string res;
		for (int i = 0; i < 16; i++) {
			if (dash[i])
				res += "-";
			res += v[dist(rng)];
			res += v[dist(rng)];
		}
		return res;
	}

	inline std::string FormIDToString(RE::FormID a_id)
	{
		std::stringstream stream;
		stream << std::hex << a_id;
		std::string str(stream.str());

		std::transform(str.begin(), str.end(), str.begin(), [](unsigned char c) { return (char)std::toupper(c); });

		return "0x" + str;
	}

	inline RE::TESObjectREFR* GetRef(RE::ObjectRefHandle a_handle)
	{
		if (a_handle) {
			if (const auto& targetPtr = a_handle.get()) {
				return targetPtr.get();
			}
		}

		return nullptr;
	}
}