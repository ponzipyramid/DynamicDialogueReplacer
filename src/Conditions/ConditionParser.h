#pragma once

#include "ConditionUtil.h"

// stolen from DAV (https://github.com/Exit-9B/DynamicArmorVariants)
namespace Conditions
{
	class ConditionParser
	{
	public:
		using RefMap = std::unordered_map<std::string, RE::TESForm*>;

		ConditionParser() = delete;

		static auto Parse(std::string_view a_text, const RefMap& a_refs) -> RE::TESConditionItem*;

		static inline std::shared_ptr<RE::TESCondition> ParseConditions(const std::vector<std::string>& a_rawConditions, const ConditionParser::RefMap& a_refs)
		{
			auto condition = std::make_shared<RE::TESCondition>();
			RE::TESConditionItem** head = std::addressof(condition->head);
			int numConditions = 0;

			for (auto& text : a_rawConditions) {
				if (text.empty())
					continue;


				if (auto conditionItem = ConditionParser::Parse(text, a_refs)) {
					*head = conditionItem;
					head = std::addressof(conditionItem->next);
					numConditions += 1;
				} else {
					logger::info("Aborting condition parsing"sv);
					return nullptr;
				}
			}

			return numConditions ? condition : nullptr;
		}

		static inline RefMap GenerateRefMap(std::unordered_map<std::string, std::string> a_rawRefs)
		{
			ConditionParser::RefMap refMap;
			refMap["PLAYER"] = RE::PlayerCharacter::GetSingleton();

			for (const auto& [key, value] : a_rawRefs) {
				if (const auto form = ConditionUtil::GetFormFromString(value)) {
					refMap[key] = form;
				}
			}

			return refMap;
		}
	private:
		union ConditionParam
		{
			char c;
			std::int32_t i;
			float f;
			RE::TESForm* form;
			RE::BSString* str;
		};

		static auto ParseParam(
			const std::string& a_text,
			RE::SCRIPT_PARAM_TYPE a_type,
			const RefMap& a_refs) -> ConditionParam;

		template <typename T = RE::TESForm>
		static auto LookupForm(const std::string& a_text, const RefMap& a_refs) -> T*
		{
			if (auto it = a_refs.find(a_text); it != a_refs.end()) {
				return it->second->As<T>();
			}

			return ConditionUtil::GetFormFromString<T>(a_text);
		}
	};
}
