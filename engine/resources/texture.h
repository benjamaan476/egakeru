#pragma once

#include "pch.h"
#include "resource.h"
namespace egkr
{

	enum class texture_use
	{
		unknown = 0,
		map_diffuse
	};

	struct texture_properties
	{
		uint32_t id{};
		uint32_t width{};
		uint32_t height{};
		uint32_t channel_count{};

		uint32_t generation{invalid_id};
		bool has_transparency{};

		const void* data{};
	};

	class texture : public resource
	{
	public:
		using shared_ptr = std::shared_ptr<texture>;

		static shared_ptr create(const void* context, const texture_properties& properties, const uint8_t* data);
		explicit texture(const texture_properties& properties);

		virtual ~texture() = default;


		virtual void destroy() = 0;

	private:
		std::string name_{};
	};

	struct texture_map
	{
		texture::shared_ptr texture{};
		texture_use use{};
	};
}