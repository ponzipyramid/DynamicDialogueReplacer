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
		inline bool IsValid() { return _valid; }
		inline std::vector<std::string> GetHashes() { 
			return _voiceTypes 
				| std::ranges::views::transform([this](RE::BGSVoiceType* a_voiceType) { 
						return GenerateHash(_topicInfo, a_voiceType, _index); 
					}) 
				| std::ranges::to<std::vector>(); 
		}
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
		inline bool ConditionsMet(RE::Actor* a_speaker, RE::TESObjectREFR* a_target) {
			return _conditions == nullptr || _conditions->IsTrue(a_speaker, a_target ? a_target : a_speaker);
		}
	private:
		std::string _subtitle;
		std::string _path;

		std::vector<RE::BGSVoiceType*> _voiceTypes;
		RE::TESTopicInfo* _topicInfo;
		
		int _index; 
		
		RE::TESCondition* _conditions = nullptr;

		bool _valid = false;

		friend struct YAML::convert<Response>; 
	};
}

namespace YAML
{
	using namespace DDR;

	template <>
	struct convert<Response>
	{
		static bool decode(const Node& node, Response& rhs)
		{
			rhs._path = node["path"].as<std::string>("");
			rhs._subtitle = node["sub"].as<std::string>("");
			rhs._index = node["index"].as<int>(-1);

			if (rhs._index < 0)
				return true;
			
			const auto voiceTypes = node["voices"].as<std::vector<std::string>>(std::vector<std::string>{});

			rhs._voiceTypes = voiceTypes 
				| std::ranges::views::transform([](const std::string& m) {
						return RE::TESForm::LookupByEditorID<RE::BGSVoiceType>(m);
					}) 
				| std::ranges::views::filter([](const RE::BGSVoiceType* a_voice) {
						return a_voice != nullptr;
					})
				| std::ranges::to<std::vector>();
				

			if (rhs._voiceTypes.empty()) {
				logger::error("must have at least one valid voice type");
				return true;
			}

			const auto topicInfo = node["id"].as<std::string>();
			
			const auto splits = Util::Split(topicInfo, "|"sv);

			if (splits.size() != 2) {
				logger::error("topic info formatted incorrectly");
				return true;
			}

			char* p;
			const auto formId = std::strtol(splits[0].c_str(), &p, 16);

			if (*p != 0) {
				logger::error("topic info form id not valid");
			}

			rhs._topicInfo = RE::TESDataHandler::GetSingleton()->LookupForm<RE::TESTopicInfo>(formId, splits[1]);

			if (!rhs._topicInfo) {	
				logger::error("topic info does not exist");
				return true;
			} 

			rhs._valid = true;
			
			return true;
		}
	};
}