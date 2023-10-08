#include "geometry_system.h"

namespace egkr
{
	static geometry_system::unique_ptr geometry_system_{};
	bool geometry_system::create(const void* renderer_context)
	{
		geometry_system_ = std::make_unique<geometry_system>(renderer_context);
		geometry_system_->init();
		return true;
	}

	geometry_system::geometry_system(const void* renderer_context)
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
		
		const float scale{ 10.F };
		const egkr::vector<vertex_3d> vertices{ {{-0.5F * scale, -0.5F * scale, 0.F}, {0.F, 0.F} }, { {0.5F * scale, 0.5F * scale, 0.F}, {1.F,1.F} }, { {-0.5F * scale, 0.5F * scale, 0.F}, {0.F,1.F} }, { {0.5F * scale, -0.5F * scale, 0.F}, {1.F,0.F} } };

		const egkr::vector<uint32_t> indices{ 0, 1, 2, 0, 3, 1 };

		geometry_properties properties{};
		properties.indices = indices;
		properties.vertices = vertices;
		properties.name = "default";
		properties.material_name = "test_material";

		geometry_system_->default_geometry_ = geometry::create(geometry_system_->renderer_context_, properties);
		return false;
	}

	void geometry_system::shutdown()
	{
		geometry_system_->default_geometry_.reset();
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

	geometry::shared_ptr geometry_system::get_default()
	{
		return geometry_system_->default_geometry_;
	}

	geometry_properties geometry_system::generate_plane(uint32_t /*width*/, uint32_t /*height*/, uint32_t /*x_segments*/, uint32_t /*y_segments*/, uint32_t /*tile_x*/, uint32_t /*tile_y*/, std::string_view /*name*/, std::string_view /*material_name*/)
	{
		return geometry_properties();
	}
}