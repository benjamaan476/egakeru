#pragma once
#include "pch.h"

namespace egkr
{
	enum class resource_type : uint8_t
	{
		text,
		binary,
		image,
		material,
		shader,
		mesh,
		bitmap_font,
		system_font,
		custom
	};

constexpr auto RESOURCE_MAGIC = 0xdeadbeef;

	struct resource_header
	{
		uint32_t magic_number{ RESOURCE_MAGIC };
		resource_type type;
		uint8_t version{ 1 };
		uint16_t reserved{};
	};

	struct resource_properties
	{
		resource_type type{};
		std::string name{};
		std::string full_path{};
		void* data{};
	};

	struct binary_resource_properties
	{
		egkr::vector<uint8_t> data;
	};

	struct image_resource_parameters
	{
		bool flip_y{};
	};

	class resource
	{
	public:
		using shared_ptr = std::shared_ptr<resource>;
		static shared_ptr create(const resource_properties& properties);

		explicit resource(const resource_properties& properties);
		resource(uint32_t id, uint32_t generation, std::string_view name) : id_{ id }, generation_{ generation }, name_{name} {}

		const auto& get_id() const { return id_; }
		void set_id(uint32_t id) { id_ = id; }


		const auto& get_generation() const { return generation_; }
		void increment_generation()
		{
			if (generation_ == invalid_32_id)
			{
				generation_ = 0;
			}
			else
			{
				++generation_;
			}
		}
		void set_generation(uint32_t generation) { generation_ = generation; }

		const auto& get_type() const { return resource_type_; }

		const auto& get_name() const { return name_; }

		void* data{};

	private:
		uint32_t id_{ invalid_32_id };
		uint32_t generation_{ invalid_32_id };

		std::string name_{};
		std::string full_path_{};

		resource_type resource_type_{};
		// Renderer specific data
	};
}
