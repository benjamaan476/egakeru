#pragma once

#include "pch.h"
#include "resource.h"
#include "material.h"

#include "renderer/vertex_types.h"

namespace egkr
{
	struct geometry_properties
	{
		uint32_t id{};
		uint32_t generation{};

		egkr::vector<vertex_3d> vertices{};
		egkr::vector<uint32_t> indices{};

		std::string name{};
		std::string material_name{};
	};

	class geometry : public resource
	{
	public:
		using shared_ptr = std::shared_ptr<geometry>;
		static shared_ptr create(const void* context, const geometry_properties& properties);

		explicit geometry(const geometry_properties& properties);
		virtual ~geometry();

		virtual void draw() const = 0;

		const auto& get_material() const { return material_;}

	private:
		uint32_t id_{invalid_id};
		uint32_t generation_{invalid_id};

		std::string name_{};
		material::shared_ptr material_{};
	};
}
