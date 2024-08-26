#pragma once

#include <unordered_set>
#include "DialogueManager.h"

using namespace RE;
namespace DDR
{
	typedef int64_t(WINAPI* SetupTopicType)(int64_t a1, int64_t a2, TESTopic* a3, int64_t a4, int64_t a5, char a6);

	class Hooks
	{
	public:
		static void Install();
	private:				
		static int64_t PopulateTopicInfo1(int64_t a_1, TESTopic* a_2, TESTopicInfo* a_3, Character* a_4, RE::TESTopicInfo::ResponseData* a_5);
		static int64_t PopulateTopicInfo2(int64_t a_1, TESTopic* a_2, TESTopicInfo* a_3, Character* a_4, RE::TESTopicInfo::ResponseData* a_5);
		static int64_t PopulateTopicInfo3(int64_t a_1, TESTopic* a_2, TESTopicInfo* a_3, Character* a_4, RE::TESTopicInfo::ResponseData* a_5);

		static inline REL::Relocation<decltype(PopulateTopicInfo1)> _PopulateTopicInfo;


		static int64_t SetupTopic(int64_t a1, int64_t a2, TESTopic* a3, int64_t a4, int64_t a5, char a6);
		static inline SetupTopicType _SetupTopic;
	};
}