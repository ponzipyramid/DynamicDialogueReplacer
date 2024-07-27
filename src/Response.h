#pragma once

#include "Util.h"
#include "ConditionParser.h"

namespace DDR
{
	class Response
	{
	public:
		static inline std::string GenerateHash(RE::TESTopicInfo* a_topicInfo, RE::BGSVoiceType* a_voiceType, int index)
		{
			if (!a_topicInfo || !a_voiceType || index < 1)
				return "";

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
		inline bool InitConditions(ConditionParser::RefMap a_refs)
		{
			auto condition = std::make_shared<RE::TESCondition>();
			RE::TESConditionItem** head = std::addressof(condition->head);
			int numConditions = 0;

			for (auto& text : _rawConditions) {
				if (text.empty())
					continue;

				if (auto conditionItem = ConditionParser::Parse(text, a_refs)) {
					*head = conditionItem;
					head = std::addressof(conditionItem->next);
					numConditions += 1;
				} else {
					logger::info("Aborting condition parsing"sv);
					return false;
				}
			}

			_conditions = numConditions ? condition : nullptr;
			_rawConditions.clear();

			return true;
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
		
		std::vector<std::string> _rawConditions;
		std::shared_ptr<RE::TESCondition> _conditions = nullptr;

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
			rhs._index = node["index"].as<int>(0);
			rhs._rawConditions = node["conditions"].as<std::vector<std::string>>(std::vector<std::string>{});

			if (rhs._index <= 0) {
				logger::info("index is required and must be greater than zero");
				return true;
			}
			
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
			rhs._topicInfo = Util::GetFormFromString<RE::TESTopicInfo>(topicInfo);

			if (!rhs._topicInfo) {	
				logger::error("topic info does not exist");
				return true;
			} 

			rhs._valid = true;
			
			return true;
		}
	};
}