#pragma once

#include "pch.h"
#include "resource.h"
#include "material.h"

#include "renderer/vertex_types.h"

namespace egkr
{
	class renderer_backend;
	namespace geometry
	{
		struct properties
		{
			uint32_t id{};
			uint32_t generation{};

			uint32_t vertex_size{};
			uint32_t vertex_count{};
			void* vertices{};

			egkr::vector<uint32_t> indices{};

			std::string name{};
			std::string material_name{ "default" };

			float3 center{};
			float3 min_extent{};
			float3 max_extent{};

			void release()
			{
				free(vertices);
			}

			~properties()
			{
				//Dynamically allocated so needs to be freed
			}
		};

		struct render_data;
		class geometry : public resource
		{
		public:
			using shared_ptr = std::shared_ptr<geometry>;
			static shared_ptr create(const renderer_backend* backend, const properties& properties);

			geometry(const renderer_backend* backend, const properties& properties);
			virtual ~geometry() = default;

			virtual bool populate(const properties& properties) = 0;
			virtual void draw() = 0;
			virtual void free() = 0;
			void destroy();

			const auto& get_material() const { return material_; }
			void set_material(const material::shared_ptr& material);

		protected:
			material::shared_ptr material_{};
			const renderer_backend* backend_{};
		};

		struct render_data
		{
			geometry::shared_ptr geometry{};
			float4x4 model{};
		};
	}
}
