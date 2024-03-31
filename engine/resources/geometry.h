#pragma once

#include "pch.h"
#include "resource.h"
#include "material.h"
#include "transform.h"

#include "renderer/vertex_types.h"

namespace egkr
{
	class geometry : public resource
	{
	public:
		struct properties
		{
			std::string name{};
			uint32_t id{};
			uint32_t generation{};

			uint32_t vertex_size{};
			uint32_t vertex_count{};
			void* vertices{};

			egkr::vector<uint32_t> indices{};

			std::string material_name{ default_material_name_ };

			float3 center{};
			extent3d extents{};
			void release()
			{
				::free(vertices);
			}

			~properties()
			{
				//Dynamically allocated so needs to be freed
			}
		};
		using shared_ptr = std::shared_ptr<geometry>;
		static shared_ptr create(const properties& properties);

		explicit geometry(const properties& properties);
		virtual ~geometry() = default;

		virtual bool populate(const properties& properties) = 0;
		virtual void draw() = 0;
		virtual void update_vertices(uint32_t offset, uint32_t vertex_count, void* vertices) = 0;
		virtual void free() = 0;
		void destroy();

		[[nodiscard]] const auto& get_properties() const
		{
			return properties_;
		}
		[[nodiscard]] const auto& get_material() const
		{
			return material_;
		}
		void set_material(const material::shared_ptr& material);

	protected:
		properties properties_{};
		material::shared_ptr material_{};
	};

	struct render_data
	{
		geometry::shared_ptr geometry{};
		transform model{};
		uint64_t unique_id{};
	};

	struct frame_data
	{
		egkr::vector<render_data> world_geometries{};
		egkr::vector<render_data> debug_geometries{};

		void reset()
		{
			world_geometries.clear();
			debug_geometries.clear();
		}
	};
}