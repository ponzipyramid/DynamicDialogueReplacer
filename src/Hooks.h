#pragma once

#include <unordered_set>
#include "DialogueManager.h"

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

	class DialogueMenuEx : public RE::DialogueMenu
	{
	public:
		static inline void Install()
		{
			REL::Relocation<uintptr_t> vtbl(RE::VTABLE_DialogueMenu[0]);
			_ProcessMessageFn = vtbl.write_vfunc(0x4, &ProcessMessageEx);
		}

	public:
		RE::UI_MESSAGE_RESULTS ProcessMessageEx(RE::UIMessage& a_message);

	private:
		using ProcessMessageFn = decltype(&RE::DialogueMenu::ProcessMessage);
		static inline REL::Relocation<ProcessMessageFn> _ProcessMessageFn;

		static inline RE::TESObjectREFR* _currentTarget;
		static inline std::unordered_map<RE::FormID, Topic*> _cache;
	};
}