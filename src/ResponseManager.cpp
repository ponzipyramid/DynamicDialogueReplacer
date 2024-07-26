#include "ResponseManager.h"

using namespace DDR;

void ResponseManager::Init()
{
	if (_init)
		return;

	_init = true;

	// TODO: parse all files in DynamicDialogueReplacer folder
}

Response* ResponseManager::FindReplacement(RE::Character* a_speaker, RE::TESTopicInfo* a_topicInfo, RE::BGSVoiceType* a_voiceType, RE::TESTopicInfo::ResponseData* a_responseData)
{
	const auto key = Response::GenerateHash(a_topicInfo, a_voiceType, a_responseData->responseNumber);

	if (_replacements.count(key)) {
		const auto& replacements = _replacements[key];
		for (const auto& repl : replacements) {
			RE::TESObjectREFR* target = nullptr;

			if (const auto& targetHandle = a_speaker->GetActorRuntimeData().dialogueItemTarget) {
				if (const auto& targetPtr = targetHandle.get()) {
					target = targetPtr.get();
				}
			}

			if (repl->IsValid(a_speaker, target))
				return repl;
		}
	}

	return nullptr;
}