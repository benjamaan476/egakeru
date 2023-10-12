#pragma once
#include "pch.h"

namespace egkr
{
	enum class resource_type
	{
		text,
		binary,
		image,
		material,
		stataic_mesh,
		custom
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

	class resource
	{
	public:
		using shared_ptr = std::shared_ptr<resource>;
		static shared_ptr create(const resource_properties& properties);

		explicit resource(const resource_properties& properties);
		resource(uint32_t id, uint32_t generation) : id_{ id }, generation_{ generation } {}

		const auto& get_id() const { return id_; }
		void set_id(uint32_t id) { id_ = id; }


		const auto& get_generation() const { return generation_; }
		void increment_generation() { ++generation_; }
		void set_generation(uint32_t generation) { generation_ = generation; }

		const auto& get_type() const { return resource_type_; }

		void* data{};

	private:
		uint32_t id_{ invalid_id };
		uint32_t generation_{ invalid_id };

		std::string name_{};
		std::string full_path_{};

		resource_type resource_type_{};
		// Renderer specific data
	};
}
