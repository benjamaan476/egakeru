#include "geometry_system.h"

namespace egkr
{
	static geometry_system::unique_ptr geometry_system_{};
	bool geometry_system::create(const renderer_frontend* renderer_context)
	{
		geometry_system_ = std::make_unique<geometry_system>(renderer_context);
		geometry_system_->init();
		return true;
	}

	geometry_system::geometry_system(const renderer_frontend* renderer_context)
		: renderer_context_{ renderer_context }, max_geometry_count_{ 4096 }
	{
	}

	geometry_system::~geometry_system()
	{
		shutdown();
	}

	bool geometry_system::init()
	{
		if (geometry_system_->max_geometry_count_ == 0)
		{
			LOG_FATAL("Material max count must be > 0");
			return false;
		}

		geometry_system_->registered_geometries_.reserve(geometry_system_->max_geometry_count_);
		

		const auto properties = generate_plane(10, 5, 8, 4, 2, 1, "default", "test_material");

		geometry_system_->default_geometry_ = geometry::create(geometry_system_->renderer_context_, properties);

		return true;
	}

	void geometry_system::shutdown()
	{
		if (geometry_system_->default_geometry_)
		{
			geometry_system_->renderer_context_->free_geometry(geometry_system_->default_geometry_.get());
			geometry_system_->default_geometry_.reset();
		}

		for (auto geometry : geometry_system_->registered_geometries_)
		{
			geometry_system_->renderer_context_->free_geometry(geometry.get());
		}
		geometry_system_->registered_geometries_.clear();

	}

	geometry::shared_ptr geometry_system::acquire(uint32_t /*id*/)
	{
		return geometry::shared_ptr();
	}

	geometry::shared_ptr geometry_system::acquire(const geometry_properties& /*properties*/)
	{
		return geometry::shared_ptr();
	}

	void geometry_system::release_geometry(const geometry::shared_ptr& geometry)
	{
		geometry_system_->renderer_context_->free_geometry(geometry.get());
	}

	geometry::shared_ptr geometry_system::get_default()
	{
		return geometry_system_->default_geometry_;
	}

	geometry_properties geometry_system::generate_plane(uint32_t width, uint32_t height, uint32_t x_segments, uint32_t y_segments, uint32_t tile_x, uint32_t tile_y, std::string_view name, std::string_view material_name)
	{
		geometry_properties plane_properties{};
		plane_properties.name = name;
		plane_properties.material_name = material_name;

		egkr::vector<vertex_3d> vertices{ (x_segments + 1) * (y_segments + 1) };
		egkr::vector<uint32_t> indices(6 * x_segments * y_segments);

		uint32_t index{ 0 };
		//Posts, not gaps
		for (auto y{ 0U }; y < y_segments + 1; ++y)
		{
			auto y_pos = 2 * y / (float)y_segments - 1;

			auto tex_y = y / (float)y_segments;
			tex_y *= tile_y;

			for (auto x{ 0U }; x < x_segments + 1; ++x)
			{
				auto x_pos = 2 * x / (float)x_segments - 1;
				auto tex_x = x / (float)x_segments;
				tex_x *= tile_x;

				vertices[index] = { {width * x_pos, height * y_pos, 0.F},  {tex_x, tex_y} };
				++index;
			}
		}

		plane_properties.vertex_count = vertices.size();
		plane_properties.vertex_size = sizeof(vertex_3d);

		auto size = plane_properties.vertex_count * plane_properties.vertex_size;

		plane_properties.vertices = malloc(size);

		auto* new_verts = (vertex_3d*)plane_properties.vertices;

		std::copy(vertices.data(), vertices.data() + plane_properties.vertex_count, new_verts);


		index = 0;
		for (auto y_index{ 0U }; y_index < y_segments; ++y_index)
		{
			for (auto x_index{ 0U }; x_index < x_segments; ++x_index)
			{
				auto i = y_index * (x_segments + 1) + x_index;

				indices[index++] = i;
				indices[index++] = i + 1;
				indices[index++] = i + (x_segments + 1) + 1;

				indices[index++] = i;
				indices[index++] = i + (x_segments + 1) + 1;
				indices[index++] = i + (x_segments + 1);

			}
		}

		plane_properties.indices = indices;

		return plane_properties;
	}
}