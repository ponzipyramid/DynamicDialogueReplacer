#pragma once

#include "DialogueManager.h"

using namespace DDR;

namespace
{
	constexpr std::string_view PapyrusClass = "DynamicDialogueReplacer";

	std::string AddReplacementTopic(RE::StaticFunctionTag*, RE::FormID a_topicId, std::string a_text)
	{
		return DialogueManager::AddReplacementTopic(a_topicId, a_text);
	}
	void RemoveReplacementTopic(RE::StaticFunctionTag*, RE::FormID a_topicId, std::string a_key)
	{
		return DialogueManager::RemoveReplacementTopic(a_topicId, a_key);
	}
}

namespace DDR::Papyrus
{
	inline bool RegisterFunctions(RE::BSScript::IVirtualMachine* vm)
	{
#define REGISTERPAPYRUSFUNC(name) vm->RegisterFunction(#name, PapyrusClass, name);
		
		REGISTERPAPYRUSFUNC(AddReplacementTopic)
		REGISTERPAPYRUSFUNC(RemoveReplacementTopic)

		return true;
	}
}