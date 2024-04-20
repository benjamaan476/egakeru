#include "geometry_system.h"
#include "geometry_utils.h"
#include "renderer/renderer_frontend.h"

namespace egkr
{
	static geometry_system::unique_ptr geometry_system_{};
	geometry_system* geometry_system::create()
	{
		geometry_system_ = std::make_unique<geometry_system>();
		return geometry_system_.get();
	}

	geometry_system::geometry_system()
		: max_geometry_count_{ 4096 }
	{
	}

	geometry_system::~geometry_system()
	{
		shutdown();
	}

	bool geometry_system::init()
	{
		if (max_geometry_count_ == 0)
		{
			LOG_FATAL("Material max count must be > 0");
			return false;
		}

		registered_geometries_.reserve(max_geometry_count_);
		
		auto properties = generate_cube(10, 10, 10, 1, 1, "default", "test_material");
		generate_tangents(properties.vertices, properties.indices);
		default_geometry_ = geometry::geometry::create(properties);

		return true;
	}

	bool geometry_system::shutdown()
	{
		if (geometry_system_->default_geometry_)
		{
			geometry_system_->default_geometry_->free();
			geometry_system_->default_geometry_.reset();
		}

		for (auto geometry : geometry_system_->registered_geometries_)
		{
			geometry->free();
		}
		geometry_system_->registered_geometries_.clear();
		return true;
	}

	geometry::geometry::shared_ptr geometry_system::acquire(uint32_t /*id*/)
	{
		return geometry::geometry::shared_ptr();
	}

	geometry::geometry::shared_ptr geometry_system::acquire(const geometry::properties& properties)
	{
		return geometry::geometry::create(properties);
	}

	void geometry_system::release_geometry(const geometry::geometry::shared_ptr& geometry)
	{
		geometry->free();
	}

	geometry::geometry::shared_ptr geometry_system::get_default()
	{
		return geometry_system_->default_geometry_;
	}

	geometry::properties geometry_system::generate_plane(uint32_t width, uint32_t height, uint32_t x_segments, uint32_t y_segments, uint32_t tile_x, uint32_t tile_y, std::string_view name, std::string_view material_name)
	{
		geometry::properties plane_properties{};
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

				vertices[index] = { {width * x_pos, height * y_pos, 0.F}, {0.F, 0.F, 1.F}, {tex_x, tex_y} };
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
	geometry::properties geometry_system::generate_cube(float width, float height, float depth, uint32_t tile_x, uint32_t tile_y, std::string_view name, std::string_view material_name)
	{
		geometry::properties properties{};
		properties.name = name;
		properties.material_name = material_name;

		std::vector<vertex_3d> vertices{};
		std::vector<uint32_t> indices{};

		const float half_width{ width / 2 };
		const float half_height{ height / 2 };
		const float half_depth{ depth / 2 };
		//TOP
		{
			vertices.emplace_back(float3{ -half_width, -half_height, half_depth }, float3{ 0, 0, 1 }, float2{ 0, 0 });
			vertices.emplace_back(float3{ half_width, -half_height, half_depth }, float3{ 0, 0, 1 }, float2{ tile_x, 0 });
			vertices.emplace_back(float3{ half_width, half_height, half_depth }, float3{ 0, 0, 1 }, float2{ tile_x, tile_y });
			vertices.emplace_back(float3{ -half_width, half_height, half_depth }, float3{ 0, 0, 1 }, float2{ 0, tile_y });

			indices.push_back(0);
			indices.push_back(1);
			indices.push_back(2);
			indices.push_back(0);
			indices.push_back(2);
			indices.push_back(3);
		}

		//BOTTOM
		{
			vertices.emplace_back(float3{ half_width, -half_height, -half_depth }, float3{ 0, 0, -1 }, float2{ 0, 0 });
			vertices.emplace_back(float3{ -half_width, -half_height, -half_depth }, float3{ 0, 0, -1 }, float2{ tile_x, 0 });
			vertices.emplace_back(float3{ -half_width, half_height, -half_depth }, float3{ 0, 0, -1 }, float2{ tile_x, tile_y });
			vertices.emplace_back(float3{ half_width, half_height, -half_depth }, float3{ 0, 0, -1 }, float2{ 0, tile_y });

			indices.push_back(4 + 0);
			indices.push_back(4 + 1);
			indices.push_back(4 + 2);
			indices.push_back(4 + 0);
			indices.push_back(4 + 2);
			indices.push_back(4 + 3);
		}

		//LEFT
		{
			vertices.emplace_back(float3{ -half_width, half_height, -half_depth }, float3{ -1, 0, 0 }, float2{ 0, 0 });
			vertices.emplace_back(float3{ -half_width, -half_height, -half_depth }, float3{ -1, 0, 0 }, float2{ tile_x, 0 });
			vertices.emplace_back(float3{ -half_width, -half_height, half_depth }, float3{ -1, 0, 0 }, float2{ tile_x, tile_y });
			vertices.emplace_back(float3{ -half_width, half_height, half_depth }, float3{ -1, 0, 0 }, float2{ 0, tile_y });

			indices.push_back(8 + 0);
			indices.push_back(8 + 1);
			indices.push_back(8 + 2);
			indices.push_back(8 + 0);
			indices.push_back(8 + 2);
			indices.push_back(8 + 3);
		}

		//RIGHT
		{
			vertices.emplace_back(float3{ half_width, -half_height, -half_depth }, float3{ 1, 0, 0 }, float2{ 0, 0 });
			vertices.emplace_back(float3{ half_width, half_height, -half_depth }, float3{ 1, 0, 0 }, float2{ tile_x, 0 });
			vertices.emplace_back(float3{ half_width, half_height, half_depth }, float3{ 1, 0, 0 }, float2{ tile_x, tile_y });
			vertices.emplace_back(float3{ half_width, -half_height, half_depth }, float3{ 1, 0, 0 }, float2{ 0, tile_y });

			indices.push_back(12 + 0);
			indices.push_back(12 + 1);
			indices.push_back(12 + 2);
			indices.push_back(12 + 0);
			indices.push_back(12 + 2);
			indices.push_back(12 + 3);
		}

		//FRONT
		{
			vertices.emplace_back(float3{ half_width, half_height, -half_depth }, float3{ 0, 1, 0 }, float2{ 0, 0 });
			vertices.emplace_back(float3{ -half_width, half_height, -half_depth }, float3{ 0, 1, 0 }, float2{ tile_x, 0 });
			vertices.emplace_back(float3{ -half_width, half_height, half_depth }, float3{ 0, 1, 0 }, float2{ tile_x, tile_y });
			vertices.emplace_back(float3{ half_width, half_height, half_depth }, float3{ 0, 1, 0 }, float2{ 0, tile_y });

			indices.push_back(16  + 0);
			indices.push_back(16  + 1);
			indices.push_back(16  + 2);
			indices.push_back(16  + 0);
			indices.push_back(16  + 2);
			indices.push_back(16  + 3);
		}

		//BACK
		{
			vertices.emplace_back(float3{ -half_width, -half_height, -half_depth }, float3{ 0, -1, 0 }, float2{ 0, 0 });
			vertices.emplace_back(float3{ half_width, -half_height, -half_depth }, float3{ 0, -1, 0 }, float2{ tile_x, 0 });
			vertices.emplace_back(float3{ half_width, -half_height, half_depth }, float3{ 0, -1, 0 }, float2{ tile_x, tile_y });
			vertices.emplace_back(float3{ -half_width, -half_height, half_depth }, float3{ 0, -1, 0 }, float2{ 0, tile_y });

			indices.push_back(20 + 0);
			indices.push_back(20 + 1);
			indices.push_back(20 + 2);
			indices.push_back(20 + 0);
			indices.push_back(20 + 2);
			indices.push_back(20 + 3);
		}


		properties.indices = indices;
		properties.vertex_count = vertices.size();
		properties.vertex_size = sizeof(vertex_3d);

		auto size = properties.vertex_count * properties.vertex_size;
		properties.vertices = malloc(size);

		auto* new_verts = (vertex_3d*)properties.vertices;

		std::copy(vertices.data(), vertices.data() + properties.vertex_count, new_verts);

		generate_tangents(properties.vertices, properties.indices);

		properties.extents.min = { -half_width, -half_depth, -half_height };
		properties.extents.max = { half_width, half_depth, half_height };
		return properties;
	}
}