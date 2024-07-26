#pragma once
#include <unordered_set>

using namespace RE;
namespace DDR
{
	typedef int64_t(WINAPI* PopulateTopicInfoType)(int64_t a_1, TESTopic* a_2, TESTopicInfo* a_3, Character* a_4, int64_t a_5);

	class Hooks
	{
	public:
		static void Install();
	private:
		static int64_t PopulateTopicInfo(int64_t a_1, TESTopic* a_2, TESTopicInfo* a_3, Character* a_4, int64_t a_5);
		static inline PopulateTopicInfoType _PopulateTopicInfo;
		
		static char* SetSubtitle(DialogueResponse* a_response, char* text, int32_t unk);
		static inline REL::Relocation<decltype(SetSubtitle)> _SetSubtitle;

		static bool ConstructResponse(TESTopicInfo::ResponseData* a_response, char* a_filePath, BGSVoiceType* a_voiceType, TESTopic* a_topic, TESTopicInfo* a_topicInfo);
		static inline REL::Relocation<decltype(ConstructResponse)> _ConstructResponse;

		static inline std::string test = "Data\\Sound\\FX\\sob_female2.wav";

		thread_local static inline RE::TESTopic* _topic = nullptr;
		thread_local static inline RE::TESTopicInfo* _topicInfo = nullptr;
		thread_local static inline RE::Character* _speaker = nullptr;
	};
}