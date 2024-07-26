#pragma once

#include "Response.h"

namespace DDR
{
	class DialogueManager
	{
	private:
		std::unordered_map<std::string, Response> _responses;
	};
}