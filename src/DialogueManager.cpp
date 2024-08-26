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

				for (auto& rr : respReplacements) {
					const auto repl = std::make_shared<Response>(rr);
					if (repl->IsValid() && repl->Init(refMap)) {
						_responses.emplace_back(repl);

						for (const auto& hash : repl->GetHashes()) {
							_respReplacements[hash].emplace_back(_responses[_responses.size() - 1]);
						}
					} else {
						logger::info("replacement not valid");
					}
				}

				auto topicReplacements = file["topics"].as<std::vector<Topic>>(std::vector<Topic>{});

				for (auto& tr : topicReplacements) {
					const auto repl = std::make_shared<Topic>(tr);

					if (repl->IsValid() && repl->InitConditions(refMap)) {
						_topics.emplace_back(repl);
						_topicReplacements[repl->GetId()].emplace_back(_topics[_topics.size() - 1]);
					} else {
						logger::info("replacement not valid");
					}
				}

				logger::info("loaded {} response replacements and {} topic replacements from {}", _respReplacements.size(), _topicReplacements.size(), fileName);

			} catch (std::exception& e) {
				logger::info("failed to load {} - {}", fileName, e.what());
			}
		}
	} else {
		logger::info("replacement dir does not exist");
	}
}

std::shared_ptr<Response> DialogueManager::FindReplacementResponse(RE::Character* a_speaker, RE::TESTopicInfo* a_topicInfo, RE::TESTopicInfo::ResponseData* a_responseData)
{
	if (!a_topicInfo || !a_responseData) {
		return nullptr;
	}

	RE::Actor* speaker = a_speaker;
	RE::TESObjectREFR* target = nullptr;
	RE::BGSVoiceType* voiceType = nullptr;
	if (speaker) {
		// regular convo between actors
		if (const auto base = speaker->GetActorBase()) {
			voiceType = base->GetVoiceType();
		}

		if (const auto& targetHandle = a_speaker->GetActorRuntimeData().dialogueItemTarget) {
			if (const auto& targetPtr = targetHandle.get()) {
				target = targetPtr.get();
			}
		}
	}

	if (!speaker || !voiceType) {
		return nullptr;
	}

	auto key = Response::GenerateHash(a_topicInfo->GetFormID(), voiceType, a_responseData->responseNumber);

	auto iter = _respReplacements.find(key);
	if (iter == _respReplacements.end()) {
		// try universal instead
		key = Response::GenerateHash(a_topicInfo->GetFormID(), a_responseData->responseNumber);
		iter = _respReplacements.find(key);
	}

	if (iter != _respReplacements.end()) {
		const auto& replacements = iter->second;

		for (const auto& repl : replacements) {
			if (repl->ConditionsMet(speaker, target))
				return repl;
		}
	}

	return nullptr;
}

std::shared_ptr<Topic> DialogueManager::FindReplacementTopic(RE::FormID a_id, RE::TESObjectREFR* a_target)
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