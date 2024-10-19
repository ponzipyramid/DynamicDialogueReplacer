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

	_AddTopic = trampoline.write_call<5>(REL::Relocation<std::uintptr_t>{ REL::VariantID{ 34460, 35287, 0x05766F0 } }.address() + REL::Relocate(0xFA, 0x154), AddTopic);
	trampoline.write_call<5>(REL::Relocation<std::uintptr_t>{ REL::VariantID{ 34477, 35304, 0x05778D0 } }.address() + REL::Relocate(0x79, 0x6C), AddTopic);

	DialogueMenuEx::Install();

	logger::info("installed hooks");
}

int64_t Hooks::PopulateTopicInfo(int64_t a_1, TESTopic* a_2, TESTopicInfo* a_3, Character* a_4, RE::TESTopicInfo::ResponseData* a_5)
{
	_responseNumber = a_5->responseNumber;
	if (_responseNumber == 1) {
		_response = DialogueManager::FindReplacementResponse(a_4, a_3, a_5);
	}

	if (_response && _response->ShouldCut(_responseNumber)) {
		delete a_5->next;
		a_5->next = nullptr;
	}

	return _PopulateTopicInfo(a_1, a_2, a_3, a_4, a_5);
}

char* Hooks::SetSubtitle(DialogueResponse* a_response, char* a_text, int32_t a_3)
{
	std::string text{ (_response && _response->HasReplacementSub(_responseNumber)) ? _response->GetSubtitle(_responseNumber) : a_text };

	//logger::info("SetSubtitle - {} - {}", a_text, text);

	return _SetSubtitle(a_response, text.data(), a_3);
}

bool Hooks::ConstructResponse(TESTopicInfo::ResponseData* a_response, char* a_filePath, BGSVoiceType* a_voiceType, TESTopic* a_topic, TESTopicInfo* a_topicInfo)
{
	if (_ConstructResponse(a_response, a_filePath, a_voiceType, a_topic, a_topicInfo)) {
		std::string filePath{ a_filePath };
		
		if (_response && _response->HasReplacementPath(_responseNumber))
		{
			//logger::info("replacing with {}", _response->GetPath(a_topic, a_topicInfo, a_voiceType, a_response->responseNumber));
			*a_filePath = NULL;
			strcat_s(a_filePath, 0x104ui64, _response->GetPath(a_topic, a_topicInfo, a_voiceType, a_response->responseNumber).c_str());
		}

		return true;
	}

	return false;
} 

int64_t Hooks::AddTopic(RE::MenuTopicManager* a_this, RE::TESTopic* a_topic, int64_t a_3, int64_t a_4)
{
	if (!a_topic)
		return _AddTopic(a_this, a_topic, a_3, a_4);

	const auto& target = Util::GetRef(a_this->speaker);

	if (const auto& resp = DialogueManager::FindReplacementTopic(a_topic->GetFormID(), target, true)) {
		bool flag = true;
		if (resp->GetCheck()) {
			flag = false;
			auto currInfo = a_topic->topicInfos;
			for (auto i = a_topic->numTopicInfos; i > 0; i--) {
				if (currInfo && *currInfo) {
					if ((*currInfo)->objConditions.IsTrue(target, RE::PlayerCharacter::GetSingleton())) {
						flag = true;
						break;
					}
				}

				currInfo++;
			}
		}
		
		if (flag) {
			// hide topic
			if (resp->IsHidden())
				return 0;

			// replace topic
			const auto& repl = resp->GetTopic();
			const auto& res = repl ? _AddTopic(a_this, repl, a_3, a_4) : 0;

			// inject additional topics
			const auto& injections = resp->GetInjections();
			for (const auto& injectTopic : injections) {
				_AddTopic(a_this, injectTopic, a_3, a_4);
			}

			if (res || !resp->ShouldProceed()) {
				return res;
			}
		}
	}

	return _AddTopic(a_this, a_topic, a_3, a_4);
}

RE::UI_MESSAGE_RESULTS DialogueMenuEx::ProcessMessageEx(RE::UIMessage& a_message)
{
	if (const auto menu = RE::MenuTopicManager::GetSingleton()) {		
		// find dialogue target on start
		if (a_message.type == RE::UI_MESSAGE_TYPE::kShow || a_message.type == RE::UI_MESSAGE_TYPE::kUpdate) {
			_currentTarget = Util::GetRef(menu->speaker);
			_currId = -1;
		}

		const auto rootId = menu->rootTopicInfo ? menu->rootTopicInfo->GetFormID() : 0;

		if (_currId == -1 || _currId != rootId) {
			_cache.clear();
			_currId = rootId;
		}

		if (const auto dialogue = menu->dialogueList) {
			#pragma warning(suppress: 4834)
			for (auto it = dialogue->begin(); it != dialogue->end(); it++) {
				if (auto curr = *it) {
					const auto id = curr->parentTopic->GetFormID();

					std::shared_ptr<Topic> replacement = nullptr;
					const auto iter = _cache.find(id);
					if (iter != _cache.end()) { // find in cache first
						replacement = iter->second;
					} else { // evaluate and place
						replacement = DialogueManager::FindReplacementTopic(id, _currentTarget, false);
						_cache[id] = replacement;
					}

					if (replacement) {
						curr->topicText = replacement->GetText();
					}
				}
			}
		}
	}

	return _ProcessMessageFn(this, a_message);
}
