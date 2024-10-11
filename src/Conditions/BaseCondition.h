#pragma once

namespace Conditions
{
	class BaseCondition
	{
	public:
		virtual bool IsTrue(RE::ConditionCheckParams& a_params) = 0;
	};
}