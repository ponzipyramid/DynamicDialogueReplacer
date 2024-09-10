#pragma once

#include <unordered_set>
#include "DialogueManager.h"

using namespace RE;

namespace RE
{
	int64_t AddTopic(RE::MenuTopicManager* a_this, RE::TESTopic* a_topic, int64_t a_3, int64_t a_4);
}

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

		static inline int64_t AddTopic(RE::MenuTopicManager* a_this, RE::TESTopic* a_topic, int64_t a_3, int64_t a_4);
		static inline REL::Relocation<decltype(AddTopic)> _AddTopic;

		thread_local static inline std::shared_ptr<TopicInfo> _response = nullptr;
		thread_local static inline int _responseNumber = -1;
	};

	class DialogueMenuEx : public RE::DialogueMenu
	{
	public:
		static inline void Install()
		{
			REL::Relocation<uintptr_t> vtbl(RE::VTABLE_DialogueMenu[0]);
			_ProcessMessageFn = vtbl.write_vfunc(0x4, &ProcessMessageEx);

			_inject = REL::Module::GetRuntime() == REL::Module::Runtime::AE;
		}

		RE::UI_MESSAGE_RESULTS ProcessMessageEx(RE::UIMessage& a_message);
		
	private:
		using ProcessMessageFn = decltype(&RE::DialogueMenu::ProcessMessage);
		static inline REL::Relocation<ProcessMessageFn> _ProcessMessageFn;

		static inline int64_t _currId;
		static inline RE::TESObjectREFR* _currentTarget;
		static inline std::unordered_map<RE::FormID, std::shared_ptr<Topic>> _cache;
		static inline bool _inject = true;
	};
}