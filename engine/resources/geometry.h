#pragma once

#include "pch.h"
#include "resource.h"
#include "material.h"
#include "resources/transform.h"

#include "renderer/vertex_types.h"

#include <renderer/renderbuffer.h>

namespace egkr
{
	struct renderable;
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
		};
		using shared_ptr = std::shared_ptr<geometry>;
		static shared_ptr create(const properties& properties);

		explicit geometry(const properties& properties);
		virtual ~geometry() = default;

		virtual bool populate(const properties& properties) = 0;
		virtual bool upload() = 0;
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

		renderbuffer::renderbuffer::shared_ptr vertex_buffer_{};
		void* vertices_{};
		uint32_t vertex_count_{};
		uint32_t vertex_size_{};
		uint32_t vertex_offset_{};

		renderbuffer::renderbuffer::shared_ptr index_buffer_{};
		egkr::vector<uint32_t> indices_{};
		uint32_t index_count_{};
		uint32_t index_size_{};
		uint32_t index_offset_{};
	};

	struct render_data
	{
		geometry::shared_ptr geometry{};
		std::weak_ptr<transformable> transform;
		uint64_t unique_id{};
		bool is_winding_reversed;
	};

	struct frame_geometry_data
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
