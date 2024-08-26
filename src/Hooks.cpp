#include "Hooks.h"

using namespace DDR;

namespace RE
{
	int64_t AddTopic(RE::MenuTopicManager* a_this, RE::TESTopic* a_topic, int64_t a_3, int64_t a_4)
	{
		// add SE address
		using func_t = decltype(&RE::AddTopic);
		REL::Relocation<func_t> func{ REL::ID(35303) };
		return func(a_this, a_topic, a_3, a_4);
	}
}

void Hooks::Install() 
{
	auto& trampoline = SKSE::GetTrampoline();
	SKSE::AllocTrampoline(64);

	REL::Relocation<std::uintptr_t> target{ REL::RelocationID(34429, 35249) };
	
	_SetSubtitle = trampoline.write_call<5>(target.address() + REL::Relocate(0x61, 0x61), SetSubtitle);
	_ConstructResponse = trampoline.write_call<5>(target.address() + REL::Relocate(0xDE, 0xDE), ConstructResponse);
	
	const uintptr_t addr = target.address();

	_PopulateTopicInfo = (PopulateTopicInfoType)addr;
	DetourTransactionBegin();
	DetourUpdateThread(GetCurrentThread());
	DetourAttach(&(PVOID&)_PopulateTopicInfo, (PBYTE)&PopulateTopicInfo);

	if (DetourTransactionCommit() != NO_ERROR) {
		logger::error("Failed to install hook on PopulateTopicInfo");
	}

	DialogueMenuEx::Install();

	logger::info("installed hooks");
}

int64_t Hooks::PopulateTopicInfo(int64_t a_1, TESTopic* a_2, TESTopicInfo* a_3, Character* a_4, RE::TESTopicInfo::ResponseData* a_5)
{
	if (a_4 && a_4->GetActorBase()) {
		_response = DialogueManager::FindReplacementResponse(a_4, a_3, a_4->GetActorBase()->GetVoiceType(), a_5);
	} else {
		_response = nullptr;
	}

	//logger::info("PopulateTopicInfo - {}", _response != nullptr);

	return _PopulateTopicInfo(a_1, a_2, a_3, a_4, a_5);
}

char* Hooks::SetSubtitle(DialogueResponse* a_response, char* a_text, int32_t a_3)
{
	std::string repl{ _response ? _response->GetSubtitle() : "" };
	std::string text{ repl.empty() ? a_text : repl };

	//logger::info("SetSubtitle - {} - {}", a_text, text);

	return _SetSubtitle(a_response, text.data(), a_3);
}

bool Hooks::ConstructResponse(TESTopicInfo::ResponseData* a_response, char* a_filePath, BGSVoiceType* a_voiceType, TESTopic* a_topic, TESTopicInfo* a_topicInfo)
{
	if (_ConstructResponse(a_response, a_filePath, a_voiceType, a_topic, a_topicInfo)) {
		std::string filePath{ a_filePath };
		
		const std::string repl{ _response ? _response->GetPath(a_topic, a_topicInfo, a_voiceType) : "" };

		if (!repl.empty())
		{
			*a_filePath = NULL;
			strcat_s(a_filePath, 0x104ui64, repl.c_str());
			//logger::info("ConstructResponse - {} - {}", filePath, a_filePath);
		}

		return true;
	} else {
		return false;
	}
}

RE::UI_MESSAGE_RESULTS DialogueMenuEx::ProcessMessageEx(RE::UIMessage& a_message)
{
	if (const auto menu = RE::MenuTopicManager::GetSingleton()) {		
		// find dialogue target on start
		if (a_message.type == RE::UI_MESSAGE_TYPE::kShow) {
			logger::info("initializing current speaker: {}", (bool)menu->speaker);

			_currentTarget = nullptr;
			if (const auto& targetHandle = menu->speaker) {
				if (const auto& targetPtr = targetHandle.get()) {
					_currentTarget = targetPtr.get();
				}
			}
			logger::info("found dialogue target: {}", _currentTarget != nullptr);

			_currId = -1;
		}

		const auto rootId = menu->rootTopicInfo ? menu->rootTopicInfo->GetFormID() : 0;

		if (_currId == -1 || _currId != rootId) {
			logger::info("clearing cache");
			_cache.clear();
			_currId = rootId;
		}

		if (const auto dialogue = menu->dialogueList) {
			#pragma warning(suppress: 4834)
			for (auto it = dialogue->begin(); it != dialogue->end(); it++) {
				if (auto curr = *it) {
					const auto id = curr->parentTopic->GetFormID();

					Topic* replacement = nullptr;
					const auto iter = _cache.find(id);
					if (iter != _cache.end()) { // find in cache first
						replacement = iter->second;
					} else { // evaluate and place
						replacement = DialogueManager::FindReplacementTopic(id, _currentTarget);
						_cache[id] = replacement;
					}

					if (replacement) {
						logger::info("found replacement, replacing");
						curr->topicText = replacement->GetText();
					}
				}
			}
		}
	}

	return _ProcessMessageFn(this, a_message);
}

