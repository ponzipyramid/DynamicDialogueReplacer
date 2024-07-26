#pragma once

#include "Response.h"

namespace DDR
{
	class ResponseManager
	{
	public:
		static void Init();
		static Response* FindReplacement(RE::Character* a_speaker, RE::TESTopicInfo* a_topicInfo, RE::BGSVoiceType* a_voiceType, RE::TESTopicInfo::ResponseData* a_responseData);
	private:
		static inline std::vector<Response> _responses;
		static inline std::unordered_map<std::string, std::vector<Response*>> _replacements;
	
		static inline bool _init = false;
	};
}