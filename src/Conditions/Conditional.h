#pragma once

#include "ConditionParser.h"

namespace Conditions
{
	class Conditional
	{
	public:
		inline bool InitConditions(const std::vector<std::string> a_raw, const ConditionParser::RefMap& a_refs)
		{
			_conditions = ConditionParser::ParseConditions(a_raw, a_refs);
			return true;
		}
		inline bool ConditionsMet(RE::TESObjectREFR* a_speaker, RE::TESObjectREFR* a_target)
		{
			return _conditions == nullptr || _conditions->IsTrue(a_speaker, a_target ? a_target : a_speaker);
		}
	private:
		std::shared_ptr<RE::TESCondition> _conditions = nullptr;
	};
}