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

		static shared_ptr create(const void* context, const texture_properties& properties, const uint8_t* data);
		explicit texture(const texture_properties& properties);

		virtual ~texture() = default;

		virtual void destroy() = 0;


		[[nodiscard]] const auto& get_generation() const { return generation_; }
		[[nodiscard]] const auto& get_id() const { return id_; }
		void set_generation(uint32_t generation);

	private:
		uint32_t id_{};
		uint32_t generation_{ invalid_id };
	};
}