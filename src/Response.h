#pragma once

#include "Util.h"

namespace DDR
{
	class Response
	{
	public:
		static inline std::string GenerateHash(RE::TESTopicInfo* a_topicInfo, RE::BGSVoiceType* a_voiceType, int index)
		{
			return std::format("{}|{}|{}", a_topicInfo->GetFormID(), a_voiceType->GetFormEditorID(), index);
		}
		inline std::string GetHash() { return _hash; }
		inline std::string GetSubtitle() { return _subtitle; }
		inline std::string GetPath(RE::TESTopic* a_topic, RE::TESTopicInfo* a_topicInfo, RE::BGSVoiceType* a_voiceType)
		{
			if (_path[0] != '$')
				return _path;

			auto sections = Util::Split(_path, "\\"sv);

			for (auto& section : sections) {
				if (section == "[VOICE_TYPE]") {
					section = a_voiceType->GetFormEditorID();
				} else if (section == "[TOPIC_MOD_FILE]") {
					section = a_topic->GetFile()->GetFilename();
				} else if (section == "[TOPIC_INFO_MOD_FILE]") {
					section = a_topicInfo->GetFile()->GetFilename();
				} else if (section == "[VOICE_MOD_FILE]") {
					section = a_voiceType->GetDescriptionOwnerFile()->GetFilename();
				}
			}

			return Util::Join(sections, "\\"sv); 
		}
		inline bool IsValid(RE::Actor* a_speaker, RE::TESObjectREFR* a_target) {
			return _conditions == nullptr || _conditions->IsTrue(a_speaker, a_target ? a_target : a_speaker);
		}
	private:
		std::string _hash;
		std::string _subtitle;
		std::string _path;
		std::vector<std::string> _voiceTypes;
		std::string _topicInfo;
		
		int _index; 
		
		RE::TESCondition* _conditions = nullptr;
	};
}