#pragma once
#include <unordered_set>

#pragma once

#include "ResponseManager.h"

using namespace RE;
namespace DDR
{
	typedef int64_t(WINAPI* PopulateTopicInfoType)(int64_t a_1, TESTopic* a_2, TESTopicInfo* a_3, Character* a_4, RE::TESTopicInfo::ResponseData* a_5);

	class Hooks
	{
	public:
		static void Install();
	private:
		static int64_t PopulateTopicInfo(int64_t a_1, TESTopic* a_2, TESTopicInfo* a_3, Character* a_4, RE::TESTopicInfo::ResponseData* a_5);
		static inline PopulateTopicInfoType _PopulateTopicInfo;
		
		static char* SetSubtitle(DialogueResponse* a_response, char* text, int32_t unk);
		static inline REL::Relocation<decltype(SetSubtitle)> _SetSubtitle;

		static bool ConstructResponse(TESTopicInfo::ResponseData* a_response, char* a_filePath, BGSVoiceType* a_voiceType, TESTopic* a_topic, TESTopicInfo* a_topicInfo);
		static inline REL::Relocation<decltype(ConstructResponse)> _ConstructResponse;
		
		thread_local static inline Response* _response = nullptr;
	};
}