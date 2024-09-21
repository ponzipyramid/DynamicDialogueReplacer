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
		inline bool ConditionsMet(RE::TESObjectREFR* a_subject, RE::TESObjectREFR* a_target)
		{
			if (!_conditions)
				return true;

			logger::info("ConditionsMet - START");

			RE::ConditionCheckParams params{ a_subject, a_target };
			auto ptr = _conditions->head;
			while (ptr) {
				bool result;
				if (ptr->data.flags.isOR) {
					result = ProgressOr(ptr, params);
				} else {
					result = IsTrue(ptr, params);
					logger::info("ProgressAND: {} - {}", (int)ptr->data.functionData.function.get(), result);
					ptr = ptr->next;
				}


				if (!result) {
					logger::info("ConditionsMet - END - failed");
					return false;
				}
			}

			logger::info("ConditionsMet - END - success");
			return true;
		}
	private:
		inline bool ProgressOr(RE::TESConditionItem*& a_item, RE::ConditionCheckParams& a_params)
		{
			bool res = false;
			bool inOR = true;

			while (a_item && inOR)
			{
				res = res || IsTrue(a_item, a_params);
				inOR = a_item->data.flags.isOR;

				logger::info("ProgressOR: {} - {} {}", (int)a_item->data.functionData.function.get(), res, inOR);

				a_item = a_item->next;
			}

			return res;
		}

		inline bool IsTrue(RE::TESConditionItem* a_item, RE::ConditionCheckParams& a_params)
		{
			// depending on type use custom logic instead
			return a_item->IsTrue(a_params);
		}

		std::shared_ptr<RE::TESCondition> _conditions = nullptr;
	};
}