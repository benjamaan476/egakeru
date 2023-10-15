#pragma once

#include "pch.h"
#include "resource.h"


namespace egkr
{

	enum class texture_use
	{
		unknown = 0,
		map_diffuse,
		map_specular,
		map_normal
	};

	struct texture_properties
	{
		std::string name{};
		uint32_t id{};
		uint32_t width{};
		uint32_t height{};
		uint32_t channel_count{};

		uint32_t generation{invalid_32_id};
		bool has_transparency{};

		const void* data{};
	};

	class renderer_frontend;
	class texture : public resource
	{
	public:
		using shared_ptr = std::shared_ptr<texture>;

		static shared_ptr create(const renderer_frontend* context, const texture_properties& properties, const uint8_t* data);
		explicit texture(const texture_properties& properties);
		~texture();

		void destroy(const renderer_frontend* renderer);
	};

	struct texture_map
	{
		texture::shared_ptr texture{};
		texture_use use{};
	};
}