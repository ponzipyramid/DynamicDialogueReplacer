#include "Hooks.h"

using namespace DDR;

namespace
{
	void PrintMenuContents()
	{
		if (const auto menu = RE::MenuTopicManager::GetSingleton()) {
			if (const auto dialogue = menu->dialogueList) {
				#pragma warning(suppress: 4834)
				for (auto it = dialogue->begin(); it != dialogue->end(); it++) {
					if (auto curr = *it) {
						logger::info("Content: {}", curr->topicText.c_str());
					}
				}
			}
		}
	}
}

void Hooks::Install() 
{
	auto& trampoline = SKSE::GetTrampoline();
	SKSE::AllocTrampoline(64);
	
	/*
	REL::Relocation<std::uintptr_t> target{ REL::RelocationID(34429, 35249) };
	
	_SetSubtitle = trampoline.write_call<5>(target.address() + REL::Relocate(0x61, 0x61), SetSubtitle);
	_ConstructResponse = trampoline.write_call<5>(target.address() + REL::Relocate(0xDE, 0xDE), ConstructResponse);

	DialogueMenuEx::Install();
	
	const uintptr_t addr = target.address();

	_PopulateTopicInfo = (PopulateTopicInfoType)addr;
	DetourTransactionBegin();
	DetourUpdateThread(GetCurrentThread());
	DetourAttach(&(PVOID&)_PopulateTopicInfo, (PBYTE)&PopulateTopicInfo);

	if (DetourTransactionCommit() != NO_ERROR) {
		logger::error("Failed to install hook on PopulateTopicInfo");
	}
	*/

	_PopulateTopicInfo = trampoline.write_call<5>(REL::Relocation<std::uintptr_t>{ REL::RelocationID(35220, 35220), REL::Relocate(0x0, 0xE4) }.address(), PopulateTopicInfo1);
	trampoline.write_call<5>(REL::Relocation<std::uintptr_t>{ REL::RelocationID(35220, 35254), REL::Relocate(0x0, 0x2ED) }.address(), PopulateTopicInfo2);
	trampoline.write_call<5>(REL::Relocation<std::uintptr_t>{ REL::RelocationID(35220, 35256), REL::Relocate(0x0, 0xCD) }.address(), PopulateTopicInfo3);

	REL::Relocation<std::uintptr_t> target{ REL::RelocationID(34429, 35254) };
	const uintptr_t addr = target.address();
	_SetupTopic = (SetupTopicType)addr;
	DetourTransactionBegin();
	DetourUpdateThread(GetCurrentThread());
	DetourAttach(&(PVOID&)_SetupTopic, (PBYTE)&SetupTopic);
	if (DetourTransactionCommit() != NO_ERROR) {
		logger::error("Failed to install hook on SetupTopic");
	}

	logger::info("installed hooks");
}

int64_t Hooks::PopulateTopicInfo1(int64_t a_1, TESTopic* a_2, TESTopicInfo* a_3, Character* a_4, RE::TESTopicInfo::ResponseData* a_5)
{
	//logger::info("PopulateTopicInfo 1: \"{}\" - \"{}\"", a_2->GetName(), a_5->responseText);

	return _PopulateTopicInfo(a_1, a_2, a_3, a_4, a_5);
}

int64_t Hooks::PopulateTopicInfo2(int64_t a_1, TESTopic* a_2, TESTopicInfo* a_3, Character* a_4, RE::TESTopicInfo::ResponseData* a_5)
{
	logger::info("PopulateTopicInfo 2: \"{}\" - \"{}\"", a_2->GetName(), a_5->responseText);

	return _PopulateTopicInfo(a_1, a_2, a_3, a_4, a_5);
}

int64_t Hooks::PopulateTopicInfo3(int64_t a_1, TESTopic* a_2, TESTopicInfo* a_3, Character* a_4, RE::TESTopicInfo::ResponseData* a_5)
{
	logger::info("PopulateTopicInfo 3: \"{}\" - \"{}\"", a_2->GetName(), a_5->responseText);

	return _PopulateTopicInfo(a_1, a_2, a_3, a_4, a_5);
}

int64_t Hooks::SetupTopic(int64_t a1, int64_t a2, TESTopic* a3, int64_t a4, int64_t a5, char a6)
{
	logger::info("----------Start----------- {}", a3 ? a3->GetName() : "");
	PrintMenuContents();
	const auto res = _SetupTopic(a1, a2, a3, a4, a5, a6);
	logger::info("----------After-----------");
	PrintMenuContents();
	logger::info("------------End-----------");

	return res;
}