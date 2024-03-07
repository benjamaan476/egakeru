#pragma once

#include <pch.h>

namespace egkr
{
	class identifier
	{
	public:
		static uint32_t acquire_unique_id(void* owner);
		static void release_id(uint32_t id);
	};
}