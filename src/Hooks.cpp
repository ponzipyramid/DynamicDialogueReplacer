#include "Hooks.h"

using namespace DDR;

namespace
{
	std::string PrintMenuContents(std::string a_pre)
	{
		std::string res {a_pre + "\n"};
		if (const auto menu = RE::MenuTopicManager::GetSingleton()) {
			if (const auto dialogue = menu->dialogueList) {
				#pragma warning(suppress: 4834)
				for (auto it = dialogue->begin(); it != dialogue->end(); it++) {
					if (auto curr = *it) {
						res += std::format("Content: {}\n", curr->parentTopic->GetFormEditorID());
					}
				}
			}
		}

		return res;
	}
}

void Hooks::Install() 
{
	auto& trampoline = SKSE::GetTrampoline();
	SKSE::AllocTrampoline(256);

	REL::Relocation<std::uintptr_t> target{ REL::ID(35303) };
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

int64_t Hooks::SetupTopic(RE::MenuTopicManager* a1, RE::TESTopic* a2, int64_t a3, int64_t a4)
{
	const auto bef = PrintMenuContents(std::format("BEFORE:  {}", a2->GetFormEditorID()));
	const auto res = _SetupTopic(a1, a2, a3, a4);
	if (res) {

		if (std::string{ a2->GetFormEditorID() } == std::string{ "DDR_BranchTopicConnected" }) {
			logger::info("patching in new topic");
			_SetupTopic(a1, RE::TESForm::LookupByEditorID<RE::TESTopic>("DDR_BranchTopicDisconnected"), a3, a4);

		}
		logger::info("SetupTopic: \"{}\"", a1->rootTopicInfo ? a1->rootTopicInfo->parentTopic->GetFormEditorID() : "none");
		const auto af = PrintMenuContents(std::format("AFTER:  {}", a2->GetFormEditorID()));
		logger::info("{}", bef);
		logger::info("{}", af);
	}

	return res;
}