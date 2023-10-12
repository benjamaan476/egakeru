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

		uint32_t vertex_size{};
		uint32_t vertex_count{};
		void* vertices{};

		egkr::vector<uint32_t> indices{};

		std::string name{};
		std::string material_name{};

		~geometry_properties()
		{
			//Dynamically allocated so needs to be freed
			free(vertices);
		}
	};

	class renderer_frontend;
	class geometry : public resource
	{
	public:
		using shared_ptr = std::shared_ptr<geometry>;
		static shared_ptr create(const renderer_frontend* context, const geometry_properties& properties);

		explicit geometry(const geometry_properties& properties);
		virtual ~geometry();

		void destroy(const renderer_frontend* renderer);

		const auto& get_material() const { return material_;}

	private:
		material::shared_ptr material_{};
	};
}
