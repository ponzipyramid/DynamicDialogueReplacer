#include "DialogueManager.h"

using namespace DDR;
using namespace Conditions;

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

				const auto refMap = ConditionParser::GenerateRefMap(file["refs"].as<std::unordered_map<std::string, std::string>>(std::unordered_map<std::string, std::string>{}));
				auto respReplacements = file["topicInfos"].as<std::vector<TopicInfo>>(std::vector<TopicInfo>{});

				for (auto& rr : respReplacements) {
					const auto repl = std::make_shared<TopicInfo>(rr);
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

					if (repl->IsValid() && repl->Init(refMap)) {
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

	for (auto& [_, replacements] : _topicReplacements) {
		std::ranges::sort(replacements, [](const auto& a, const auto& b) {
			return a->GetPriority() > b->GetPriority();
		});
	}

	for (auto& [_, replacements] : _respReplacements) {
		std::ranges::sort(replacements, [](const auto& a, const auto& b) {
			return a->GetPriority() > b->GetPriority();
		});
	}
}

std::shared_ptr<TopicInfo> DialogueManager::FindReplacementResponse(RE::Character* a_speaker, RE::TESTopicInfo* a_topicInfo, RE::TESTopicInfo::ResponseData* a_responseData)
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

	const auto key = TopicInfo::GenerateHash(a_topicInfo->GetFormID(), voiceType);
	const auto allKey = TopicInfo::GenerateHash(a_topicInfo->GetFormID());

	// try stored overrides next
	auto iter = _respReplacements.find(key);
	if (iter == _respReplacements.end()) {
		iter = _respReplacements.find(allKey);
	}

	if (iter != _respReplacements.end()) {
		const auto& replacements = iter->second;
	
		std::vector<std::shared_ptr<TopicInfo>> candidates;

		for (const auto& repl : replacements) {
			const auto rand = repl->IsRand();
			if ((candidates.empty() || (rand && repl->GetPriority() >= candidates[0]->GetPriority())) && repl->ConditionsMet(speaker, target)) {
				if (rand) {
					candidates.push_back(repl);
				} else {
					return repl;
				}
			}
		}

		if (!candidates.empty()) {
			return candidates[Util::GetRandInt(0, (int)candidates.size() - 1)];
		}
	}

	return nullptr;
}

std::shared_ptr<Topic> DialogueManager::FindReplacementTopic(RE::FormID a_id, RE::TESObjectREFR* a_target, bool a_full)
{
	if (_tempTopicMutex.try_lock()) {
		if (_tempTopicReplacements.count(a_id)) {
			return _tempTopicReplacements[a_id];
		}
		_tempTopicMutex.unlock();
	}

	if (_topicReplacements.count(a_id)) {
		const auto& replacements = _topicReplacements[a_id];

		for (const auto& repl : replacements) {
			if (repl->IsFull() == a_full && repl->ConditionsMet(RE::PlayerCharacter::GetSingleton(), a_target)) {
				return repl;
			}
		}
	}

	return nullptr;
}

std::string DialogueManager::AddReplacementTopic(RE::FormID a_topicId, std::string a_text)
{
	std::unique_lock lock{ _tempTopicMutex };

	std::string key{ Util::GenerateUUID() };

	if (_tempTopicKeys.count(a_topicId)) {
		logger::info("overwrite detected on {} - previous key = {}", a_topicId, _tempTopicKeys.count(a_topicId));
	}

	_tempTopicKeys[a_topicId] = key;
	_tempTopicReplacements[a_topicId] = std::make_shared<Topic>(a_topicId, a_text);

	return key;
}

void DialogueManager::RemoveReplacementTopic(RE::FormID a_topicId, std::string a_key)
{
	std::unique_lock lock{ _tempTopicMutex };

	if (_tempTopicKeys.count(a_topicId) && _tempTopicKeys[a_topicId] != a_key) {
		return;
	}

	_tempTopicKeys.erase(a_topicId);
	_tempTopicReplacements.erase(a_topicId);
}