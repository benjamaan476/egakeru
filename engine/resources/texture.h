#pragma once

#include "pch.h"

namespace egkr
{
	struct texture_properties
	{
		uint32_t id{};
		uint32_t width{};
		uint32_t height{};
		uint32_t channel_count{};

		uint32_t generation{invalid_id};
		bool has_transparency{};
	};

	class texture
	{
	public:
		using shared_ptr = std::shared_ptr<texture>;
		explicit texture(const texture_properties& properties);
	};
}