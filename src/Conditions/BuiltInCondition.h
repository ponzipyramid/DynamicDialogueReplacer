#pragma once

#include "BaseCondition.h"

namespace Conditions
{
	class BuiltInCondition : public virtual BaseCondition
	{
	public:
		bool IsTrue(RE::ConditionCheckParams& a_params)
		{
			return _condition->IsTrue(a_params);
		}
	private:
		RE::TESConditionItem* _condition;
	};
}