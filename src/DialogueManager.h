#pragma once

#include "Response.h"
#include "Topic.h"

namespace DDR
{
	class DialogueManager
	{
	public:
		static void Init();
		static std::shared_ptr<Response> FindReplacementResponse(RE::Character* a_speaker, RE::TESTopicInfo* a_topicInfo, RE::TESTopicInfo::ResponseData* a_responseData);
		static std::shared_ptr<Topic> FindReplacementTopic(RE::FormID a_id, RE::TESObjectREFR* a_target);

		//static std::size_t AddReplacementResponse(RE::FormID a_topicInfoId, int a_index, std::string a_voiceType, std::string a_sub, std::string a_path);
		//static void RemoveReplacementResponse(std::size_t a_id);
	private:
		static inline std::vector<std::shared_ptr<Response>> _responses;
		static inline std::vector<std::shared_ptr<Response>> _tempResponses;
		static inline std::unordered_map<std::string, std::vector<std::shared_ptr<Response>>> _respReplacements;

		static inline std::vector<std::shared_ptr<Topic>> _topics;
		static inline std::unordered_map<RE::FormID, std::vector<std::shared_ptr<Topic>>> _topicReplacements;

		static inline bool _init = false;
	};
}