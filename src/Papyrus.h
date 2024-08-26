#pragma once

namespace
{

}

namespace Papyrus
{
	inline void Install(RE::BSScript::IVirtualMachine* vm)
	{
#define REGISTERPAPYRUSFUNC(name) vm->RegisterFunction(#name, PapyrusClass, name);

	}
}