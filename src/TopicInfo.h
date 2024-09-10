#pragma once

#include "Util.h"
#include "ConditionParser.h"

namespace DDR
{
	struct Response
	{
		bool keep;
		std::string sub;
		std::string path;
	};

	class TopicInfo
	{
	public:
		TopicInfo() = default;
		inline TopicInfo(RE::FormID a_id, std::vector<Response> a_responses, std::vector<RE::BGSVoiceType*> a_voices)
		{
			_topicInfoId = a_id;
			_responses = a_responses;
			_voiceTypes = a_voices;
			_valid = true;
		}
		static inline std::string GenerateHash(RE::FormID a_topicInfoId, RE::BGSVoiceType* a_voiceType)
		{
			if (!a_voiceType)
				return "";

			return std::format("{}|{}", a_topicInfoId, a_voiceType->GetFormEditorID());
		}
		static inline std::string GenerateHash(RE::FormID a_topicInfoId)
		{
			return std::format("{}|all", a_topicInfoId);
		}
		inline bool IsValid() { return _valid; }
		inline std::vector<std::string> GetHashes() const {
			return _voiceTypes.empty() ? std::vector<std::string>{ GenerateHash(_topicInfoId) } :
				_voiceTypes
				| std::ranges::views::transform([this](RE::BGSVoiceType* a_voiceType) { 
						return GenerateHash(_topicInfoId, a_voiceType); 
					}) 
				| std::ranges::to<std::vector>();
		}
		inline bool HasReplacement(int a_number) { return a_number <= _responses.size() && !_responses[a_number - 1].keep; }
		inline std::string GetSubtitle(int a_num) { return _responses[a_num - 1].sub; }
		inline std::string GetPath(RE::TESTopic* a_topic, RE::TESTopicInfo* a_topicInfo, RE::BGSVoiceType* a_voiceType, int a_num)
		{
			const auto path{ _responses[a_num - 1].path };

			if (path[0] != '$')
				return path;

			auto sections = Util::Split(path, "\\"sv);

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
		inline bool IsRand()
		{
			return _random;
		}
		inline bool ShouldCut(int a_num) { return _cut && a_num >= _responses.size(); }
		inline uint64_t GetPriority()
		{
			return _priority;
		}
	private:
		RE::FormID _topicInfoId;
		
		std::vector<Response> _responses;
		std::vector<RE::BGSVoiceType*> _voiceTypes;
				
		uint64_t _priority;

		std::vector<std::string> _rawConditions;
		std::shared_ptr<RE::TESCondition> _conditions = nullptr;

		bool _valid = false;
		bool _random = false;
		bool _cut = true;

		friend struct YAML::convert<TopicInfo>; 
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
			rhs.sub = node["sub"].as<std::string>("");
			rhs.path = node["path"].as<std::string>("");
			rhs.keep = node["keep"].as<std::string>("") == "true";
			return true;
		}
	};

	template <>
	struct convert<TopicInfo>
	{
		static bool decode(const Node& node, TopicInfo& rhs)
		{
			const auto topicInfo = node["id"].as<std::string>();
			if (const auto parsed = Util::ParseFormId(topicInfo)) {
				const auto [formId, espName] = parsed.value();
				rhs._topicInfoId = RE::TESDataHandler::GetSingleton()->LookupFormID(formId, espName);
			} else {
				logger::info("failed to parse topic info");
				return true;
			}

			rhs._responses = node["responses"].as<std::vector<Response>>(std::vector<Response>{});

			if (rhs._responses.empty()) {
				logger::info("failed to find responses in replacement");
				return true;
			}

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

			rhs._random = node["random"].as<std::string>("") == "true";
			rhs._cut = node["cut"].as<std::string>("true") == "true";

			rhs._valid = true;
			
			return true;
		}
	};
}