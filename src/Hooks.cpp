#include "Hooks.h"

using namespace DDR;

void Hooks::Install() 
{
	auto& trampoline = SKSE::GetTrampoline();
	SKSE::AllocTrampoline(32);

	REL::Relocation<std::uintptr_t> sub{ REL::RelocationID(34429, 35249) };
	_SetSubtitle = trampoline.write_call<5>(sub.address() + REL::Relocate(0x61, 0x61), SetSubtitle);

	REL::Relocation<std::uintptr_t> cre{ REL::RelocationID(34429, 35249) };
	_ConstructResponse = trampoline.write_call<5>(cre.address() + REL::Relocate(0xDE, 0xDE), ConstructResponse);
}

char* Hooks::SetSubtitle(DialogueResponse* a_response, char* a_text, int32_t a_3)
{
	std::string text{ a_text };
	text += " [Modified]";

	return _SetSubtitle(a_response, text.data(), a_3);
}


bool Hooks::ConstructResponse(TESTopicInfo::ResponseData* a_response, char* a_filePath, BGSVoiceType* a_voiceType, TESTopic* a_topic, TESTopicInfo* a_topicInfo)
{
	if (_ConstructResponse(a_response, a_filePath, a_voiceType, a_topic, a_topicInfo)) {
		
		*a_filePath = NULL;
		strcat_s(a_filePath, 0x104ui64, test.data());

		return true;
	} else {
		return false;
	}
}