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

			RE::ConditionCheckParams params{ a_subject, a_target };
			auto ptr = _conditions->head;
			while (ptr) {
				bool result;
				if (ptr->data.flags.isOR) {
					result = ProgressOr(ptr, params);
				} else {
					result = IsTrue(ptr, params);
					ptr = ptr->next;
				}

				if (!result) {
					return false;
				}
			}

			return true;
		}
	private:
		static inline bool ProgressOr(RE::TESConditionItem*& a_item, RE::ConditionCheckParams& a_params)
		{
			bool res = false;
			bool inOR = true;

			while (a_item && inOR)
			{
				res = res || IsTrue(a_item, a_params);
				inOR = a_item->data.flags.isOR;

				a_item = a_item->next;
			}

			return res;
		}

		static inline bool IsTrue(RE::TESConditionItem* a_item, RE::ConditionCheckParams& a_params)
		{
			// depending on type use custom logic instead
			const auto type = a_item->data.functionData.function.get();

			float value = 0.f;

			switch (type) {
			case RE::FUNCTION_DATA::FunctionID::kGetVMQuestVariable: {
					const auto quest = std::bit_cast<RE::TESQuest*>(a_item->data.functionData.params[0]);
					const auto scriptVar = std::bit_cast<RE::BSString*>(a_item->data.functionData.params[1]);
					const auto splits = ConditionUtil::Split(std::string{ scriptVar->c_str() }, "::");
					const auto script = splits[0];
					const auto var = splits[1];

					if (const auto ptr = ConditionUtil::Script::GetScriptObject(quest, script.c_str())) {
						value = (float)ConditionUtil::Script::GetNumProperty(ptr, var);
					}

					break;
				}
			default:
				return a_item->IsTrue(a_params);
			}

			const auto comparand = a_item->data.flags.global ? a_item->data.comparisonValue.g->value : a_item->data.comparisonValue.f;

			switch (a_item->data.flags.opCode)
			{
			case RE::CONDITION_ITEM_DATA::OpCode::kEqualTo:
				return value == comparand;
			case RE::CONDITION_ITEM_DATA::OpCode::kNotEqualTo:
				return value == comparand;
			case RE::CONDITION_ITEM_DATA::OpCode::kGreaterThan:
				return value > comparand;
			case RE::CONDITION_ITEM_DATA::OpCode::kGreaterThanOrEqualTo:
				return value >= comparand;
			case RE::CONDITION_ITEM_DATA::OpCode::kLessThan:
				return value < comparand;
			case RE::CONDITION_ITEM_DATA::OpCode::kLessThanOrEqualTo:
				return value <= comparand;
			default:
				return false;
			}
		}

		std::shared_ptr<RE::TESCondition> _conditions = nullptr;
	};
}