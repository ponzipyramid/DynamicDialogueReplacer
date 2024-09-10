#pragma once

#include "Util.h"
#include "ConditionParser.h"

namespace DDR
{
	class Response
	{
	public:
		Response() = default;
		inline Response(RE::FormID a_id, int index, std::string a_sub, std::string a_path, std::vector<RE::BGSVoiceType*> a_voices)
		{
			_topicInfoId = a_id;
			_index = index;
			_subtitle = a_sub;
			_path = a_path;
			_voiceTypes = a_voices;
			_valid = true;
		}

		static inline std::string GenerateHash(RE::FormID a_topicInfoId, RE::BGSVoiceType* a_voiceType, int index)
		{
			if (!a_voiceType || index < 1)
				return "";

			return std::format("{}|{}|{}", a_topicInfoId, a_voiceType->GetFormEditorID(), index);
		}
		static inline std::string GenerateHash(RE::FormID a_topicInfoId, int index)
		{
			if (index < 1)
				return "";

			return std::format("{}|all|{}", a_topicInfoId, index);
		}
		inline bool IsValid() { return _valid; }
		inline std::vector<std::string> GetHashes() const {
			return _voiceTypes.empty() ? std::vector<std::string>{ GenerateHash(_topicInfoId, _index) } :
				_voiceTypes
				| std::ranges::views::transform([this](RE::BGSVoiceType* a_voiceType) { 
						return GenerateHash(_topicInfoId, a_voiceType, _index); 
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
		inline bool Init(ConditionParser::RefMap& a_refs)
		{
			_conditions = ConditionParser::ParseConditions(_rawConditions, a_refs);
			_rawConditions.clear();

			return true;
		}
		inline bool ConditionsMet(RE::TESObjectREFR* a_speaker, RE::TESObjectREFR* a_target)
		{
			return _conditions == nullptr || _conditions->IsTrue(a_speaker, a_target ? a_target : a_speaker);
		}
		inline uint64_t GetPriority()
		{
			return _priority;
		}
	private:
		std::string _subtitle;
		std::string _path;

		std::vector<RE::BGSVoiceType*> _voiceTypes;
		RE::FormID _topicInfoId;
		
		int _index;
		
		uint64_t _priority;

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
			const auto topicInfo = node["id"].as<std::string>();
			if (const auto parsed = Util::ParseFormId(topicInfo)) {
				const auto [formId, espName] = parsed.value();
				rhs._topicInfoId = RE::TESDataHandler::GetSingleton()->LookupFormID(formId, espName);
			} else {
				logger::info("failed to parse topic info");
				return true;
			}

			rhs._path = node["path"].as<std::string>("");
			rhs._subtitle = node["sub"].as<std::string>("");
			rhs._index = node["index"].as<int>(1);
			rhs._rawConditions = node["conditions"].as<std::vector<std::string>>(std::vector<std::string>{});
			
			const auto voiceTypes = node["voices"].as<std::vector<std::string>>(std::vector<std::string>{});

			rhs._voiceTypes = voiceTypes 
				| std::ranges::views::transform([](const std::string& m) {
						return RE::TESForm::LookupByEditorID<RE::BGSVoiceType>(m);
					}) 
				| std::ranges::views::filter([](const RE::BGSVoiceType* a_voice) {
						return a_voice != nullptr;
					})
				| std::ranges::to<std::vector>();

			rhs._valid = true;
			
			return true;
		}
	};
}