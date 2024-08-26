#pragma once

#include <unordered_set>
#include "DialogueManager.h"

using namespace RE;
namespace DDR
{
	typedef int64_t(WINAPI* SetupTopicType)(RE::MenuTopicManager* a1, RE::TESTopic* a2, int64_t a3, int64_t a4);

	class Hooks
	{
	public:
		static void Install();
	private:
		static int64_t SetupTopic(RE::MenuTopicManager* a1, RE::TESTopic* a2, int64_t a3, int64_t a4);
		static inline SetupTopicType _SetupTopic;
	};
}