#pragma once

#include "ConditionParser.h"
#include "Util.h"

namespace DDR
{
	class Topic
	{
	public:
		Topic() = default;
		inline Topic(RE::FormID a_id, std::string a_text)
		{
			_id = a_id;
			_text = a_text;
			_valid = true;
		}
		inline RE::FormID GetId() { return _id; }
		inline bool IsValid() { return _valid; }
		inline bool InitConditions(ConditionParser::RefMap a_refs)
		{
			_conditions = ConditionParser::ParseConditions(_rawConditions, a_refs);
			_rawConditions.clear();

			return true;
		}
		inline bool ConditionsMet(RE::Actor* a_speaker, RE::TESObjectREFR* a_target)
		{
			return _conditions == nullptr || _conditions->IsTrue(a_speaker, a_target ? a_target : a_speaker);
		}
		inline const char* GetText() {
			return _text.c_str();
		}
		inline bool IsFull() { return _topic != nullptr; }
		inline RE::TESTopic* GetTopic() { return _topic; }
	private:
		RE::FormID _id = 0;
		std::string _text;
		RE::TESTopic* _topic;

		std::vector<std::string> _rawConditions;
		std::shared_ptr<RE::TESCondition> _conditions = nullptr;

		bool _valid = false;

		friend struct YAML::convert<Topic>;
	};
}

namespace YAML
{
	using namespace DDR;

	template <>
	struct convert<Topic>
	{
		static bool decode(const Node& node, Topic& rhs)
		{
			const auto id = node["id"].as<std::string>("");

			if (id.empty()) {
				logger::error("topic id must be included");
				return true;
			}

			if (const auto topic = Util::GetFormFromString<RE::TESTopic>(id)) {
				rhs._id = topic->GetFormID();
			} else {
				logger::error("topic id must be valid");
				true;
			}

			rhs._text = node["text"].as<std::string>("");

			const auto with = node["with"].as<std::string>("");
			rhs._topic = Util::GetFormFromString<RE::TESTopic>(with);

			if (rhs._text.empty() && !rhs._topic) {
				logger::error("replacement must have text or a topic {} {}", rhs._text, with);
				return true;
			}

			rhs._rawConditions = node["conditions"].as<std::vector<std::string>>(std::vector<std::string>{});

			rhs._valid = true;

			return true;
		}
	};
}