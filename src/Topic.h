#pragma once

#include "Conditions/Conditional.h"
#include "Util.h"


using namespace Conditions;

namespace DDR
{
	class Topic : public Conditional
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
		inline bool Init(const ConditionParser::RefMap& a_refs) { return InitConditions(_rawConditions, a_refs); }
		inline const char* GetText() {
			return _text.c_str();
		}
		inline bool IsHidden() { return _hide; }
		inline bool IsFull() { return _with != nullptr || _hide || !_inject.empty(); }
		inline bool ShouldProceed() { return _proceed; }
		inline RE::TESTopic* GetTopic() { return _with; }
		inline std::vector<RE::TESTopic*> GetInjections() { return _inject; }
		inline uint64_t GetPriority() { return _priority; }
		inline bool GetCheck() { return _check; }
	private:
		RE::FormID _id = 0;
		std::string _text;
		RE::TESTopic* _with;

		std::vector<RE::TESTopic*> _inject;

		std::vector<std::string> _rawConditions;

		bool _valid = false;
		bool _hide = false;
		bool _proceed = false;
		bool _check = false;

		uint64_t _priority;

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
			rhs._priority = node["priority"].as<uint64_t>(0);

			const auto with = node["with"].as<std::string>("");
			rhs._with = Util::GetFormFromString<RE::TESTopic>(with);

			const auto inject = node["inject"].as<std::vector<std::string>>(std::vector<std::string>{});
			for (const auto& str : inject) {
				if (const auto topic = Util::GetFormFromString<RE::TESTopic>(str)) {
					rhs._inject.push_back(topic);
				}
			}

			rhs._hide = node["hide"].as<std::string>("") == "true";
			rhs._proceed = node["proceed"].as<std::string>("true") == "true";
			rhs._check = node["check"].as<std::string>("") == "true";

			if (rhs._text.empty() && !rhs._with && !rhs._inject.empty()) {
				logger::error("replacement must have text or a topic {} {}", rhs._text, with);
				return true;
			}

			rhs._rawConditions = node["conditions"].as<std::vector<std::string>>(std::vector<std::string>{});

			rhs._valid = true;

			return true;
		}
	};
}