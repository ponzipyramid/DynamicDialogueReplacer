#include "DialogueManager.h"

using namespace DDR;

void DialogueManager::Init()
{
	if (_init)
		return;

	_init = true;

	// TODO: parse all files in DynamicDialogueReplacer folder

	logger::info("initializing replacements");

	std::string path{ "Data\\SKSE\\DynamicDialogueReplacer" };
	if (fs::exists(path)) {
		for (const auto& entry : fs::directory_iterator(path)) {
			if (entry.is_directory())
				continue;
			const auto ext = entry.path().extension();

			if (ext != ".yml" && ext != ".yaml")
				continue;

			const std::string fileName{ path + "\\" + entry.path().filename().string() };

			try {
				const auto file = YAML::LoadFile(fileName);

				ConditionParser::RefMap refMap;
				refMap["PLAYER"] = RE::PlayerCharacter::GetSingleton();

				const auto rawMap = file["refs"].as<std::unordered_map<std::string, std::string>>(std::unordered_map<std::string, std::string>{});

				for (const auto& [key, value] : rawMap) {
					if (const auto form = Util::GetFormFromString(value)) {
						refMap[key] = form;
					}
				}

				auto respReplacements = file["topicInfos"].as<std::vector<Response>>(std::vector<Response>{});

				for (auto& repl : respReplacements) {
					if (repl.IsValid() && repl.InitConditions(refMap)) {

						_responses.emplace_back(repl);

						for (const auto& hash : repl.GetHashes()) {
							_respReplacements[hash].emplace_back(&_responses[_responses.size() - 1]);
						}
					} else {
						logger::info("replacement not valid");
					}
				}

				auto topicReplacements = file["topics"].as<std::vector<Topic>>(std::vector<Topic>{});

				for (auto& repl : topicReplacements) {
					if (repl.IsValid() && repl.InitConditions(refMap)) {
						_topics.emplace_back(repl);
						_topicReplacements[repl.GetId()].emplace_back(&_topics[_topics.size() - 1]);
					} else {
						logger::info("replacement not valid");
					}
				}

				logger::info("loaded {} response replacements and {} topic replacements from {}", respReplacements.size(), topicReplacements.size(), fileName);

			} catch (std::exception& e) {
				logger::info("failed to load {} - {}", fileName, e.what());
			}
		}
	} else {
		logger::info("replacement dir does not exist");
	}
}

Response* DialogueManager::FindReplacementResponse(RE::Character* a_speaker, RE::TESTopicInfo* a_topicInfo, RE::BGSVoiceType* a_voiceType, RE::TESTopicInfo::ResponseData* a_responseData)
{
	if (!a_speaker || !a_topicInfo || !a_voiceType || !a_responseData)
		return nullptr;

	const auto key = Response::GenerateHash(a_topicInfo, a_voiceType, a_responseData->responseNumber);

	if (_respReplacements.count(key)) {
		const auto& replacements = _respReplacements[key];
		for (const auto& repl : replacements) {
			RE::TESObjectREFR* target = nullptr;

			if (const auto& targetHandle = a_speaker->GetActorRuntimeData().dialogueItemTarget) {
				if (const auto& targetPtr = targetHandle.get()) {
					target = targetPtr.get();
				}
			}

			if (repl->ConditionsMet(a_speaker, target))
				return repl;
		}
	}

	return nullptr;
}

Topic* DialogueManager::FindReplacementTopic(RE::FormID a_id, RE::TESObjectREFR* a_target)
{
	if (_topicReplacements.count(a_id)) {
		const auto& replacements = _topicReplacements[a_id];

		for (const auto& repl : replacements) {
			if (repl->ConditionsMet(RE::PlayerCharacter::GetSingleton(), a_target)) {
				return repl;
			}
				
		}
	}

	return nullptr;
}