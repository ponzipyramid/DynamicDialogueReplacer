#pragma once

#include "Response.h"
#include "Topic.h"

namespace DDR
{
	class DialogueManager
	{
	public:
		static void Init();
		static Response* FindReplacementResponse(RE::Character* a_speaker, RE::TESTopicInfo* a_topicInfo, RE::BGSVoiceType* a_voiceType, RE::TESTopicInfo::ResponseData* a_responseData);
		static Topic* FindReplacementTopic(RE::FormID a_id, RE::TESObjectREFR* a_target);
	private:
		static inline std::vector<Response> _responses;
		static inline std::unordered_map<std::string, std::vector<Response*>> _respReplacements;

		static inline std::vector<Topic> _topics;
		static inline std::unordered_map<RE::FormID, std::vector<Topic*>> _topicReplacements;

		static inline bool _init = false;
	};
}