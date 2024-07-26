#include "Hooks.h"

using namespace DDR;

void Hooks::Install() 
{
	auto& trampoline = SKSE::GetTrampoline();
	SKSE::AllocTrampoline(32);

	REL::Relocation<std::uintptr_t> target{ REL::RelocationID(34429, 35249) };
	
	_SetSubtitle = trampoline.write_call<5>(target.address() + REL::Relocate(0x61, 0x61), SetSubtitle);
	_ConstructResponse = trampoline.write_call<5>(target.address() + REL::Relocate(0xDE, 0xDE), ConstructResponse);
	
	const uintptr_t addr = target.address();

	_PopulateTopicInfo = (PopulateTopicInfoType)addr;
	DetourTransactionBegin();
	DetourUpdateThread(GetCurrentThread());
	DetourAttach(&(PVOID&)_PopulateTopicInfo, (PBYTE)&PopulateTopicInfo);

	if (DetourTransactionCommit() == NO_ERROR) {
		logger::info("Installed hook on PopulateTopicInfo at {0:x} with replacement from address {0:x}", addr, (void*)&PopulateTopicInfo);
	} else {
		logger::info("Failed to install hook on PopulateTopicInfo");
	}
}

int64_t Hooks::PopulateTopicInfo(int64_t a_1, TESTopic* a_2, TESTopicInfo* a_3, Character* a_4, RE::TESTopicInfo::ResponseData* a_5)
{
	_response = ResponseManager::FindReplacement(a_4, a_3, a_4->GetActorBase()->GetVoiceType(), a_5);
	logger::info("PopulateTopicInfo - {}", _response != nullptr);
	return _PopulateTopicInfo(a_1, a_2, a_3, a_4, a_5);
}

char* Hooks::SetSubtitle(DialogueResponse* a_response, char* a_text, int32_t a_3)
{
	std::string text{ _response ? _response->GetSubtitle() : a_text };
	logger::info("SetSubtitle - {} - {}", a_text, text);
	return _SetSubtitle(a_response, text.data(), a_3);
}

bool Hooks::ConstructResponse(TESTopicInfo::ResponseData* a_response, char* a_filePath, BGSVoiceType* a_voiceType, TESTopic* a_topic, TESTopicInfo* a_topicInfo)
{
	if (_ConstructResponse(a_response, a_filePath, a_voiceType, a_topic, a_topicInfo)) {
		std::string filePath{ a_filePath };
		
		if (_response)
		{
			*a_filePath = NULL;
			strcat_s(a_filePath, 0x104ui64, _response->GetPath(a_topic, a_topicInfo, a_voiceType).c_str());
		}

		logger::info("ConstructResponse - {} - {}", filePath, a_filePath);

		return true;
	} else {
		return false;
	}
}