#include "debug_box3d.h"
#include "identifier.h"

namespace egkr::debug
{
	debug_box3d::shared_ptr debug_box3d::create(const egkr::float3& size, egkr::transformable* parent)
	{
		return std::make_shared<debug_box3d>(size, parent);
	}

	debug_box3d::debug_box3d(const egkr::float3& size, egkr::transformable* parent)
		: size_{size}
	{
		set_parent(parent);
		init();
	}

	debug_box3d::~debug_box3d()
	{
		destroy();
	}

	bool debug_box3d::init()
	{
		vertices_.resize(2 * 12);
		extents_ = egkr::extent3d{ .min = -size_ * 0.5F, .max = size_ * 0.5F };
		recalculate_extents();

		unique_id_ = identifier::acquire_unique_id(this);
		return true;
	}

	bool debug_box3d::load()
	{
		geometry::properties properties{};
		properties.name = "debug_box";
		properties.vertex_count = (uint32_t)vertices_.size();
		properties.vertex_size = sizeof(colour_vertex_3d);
		properties.vertices = vertices_.data();

		geometry_ = geometry::geometry::create(properties);
		geometry_->increment_generation();

		set_colour(colour_);
		return true;
	}

	bool debug_box3d::unload()
	{
		geometry_->destroy();

		if (unique_id_ != invalid_32_id)
		{
			identifier::release_id(unique_id_);
			unique_id_ = invalid_32_id;
		}

		return true;
	}

	bool debug_box3d::update()
	{
		return true;
	}

	void debug_box3d::destroy()
	{
		if (geometry_)
		{
			unload();
		geometry_.reset();
		}

	}

	void debug_box3d::set_colour(const egkr::float4 colour)
	{
		auto col = colour;
		if (col.a == 0.F)
		{
			col.a = 1.F;
		}
		colour_ = col;

		if (geometry_)
		{
			if (geometry_->get_generation() != invalid_32_id && !vertices_.empty())
			{
				recalculate_colour();
				geometry_->update_vertices(0, (uint32_t)vertices_.size(), vertices_.data());
				geometry_->increment_generation();
			}

			if (geometry_->get_generation() == invalid_32_id)
			{
				geometry_->set_generation(0);
			}
		}
	}

	void debug_box3d::set_extents(const egkr::extent3d& extent)
	{
		if (geometry_->get_generation() != invalid_32_id && !vertices_.empty())
		{
			extents_ = extent;
			recalculate_extents();
			geometry_->update_vertices(0, (uint32_t)vertices_.size(), vertices_.data());
			geometry_->increment_generation();
		}

		if (geometry_->get_generation() == invalid_32_id)
		{
			geometry_->set_generation(0);
		}
	}

	void debug_box3d::recalculate_colour()
	{
		for (auto& vertex : vertices_)
		{
			vertex.colour = colour_;
		}
	}

	void debug_box3d::recalculate_extents()
	{
		vertices_[0].position = { extents_.min, 1.F };
		vertices_[1].position = { extents_.max.x, extents_.min.y, extents_.min.z, 1.F };
		vertices_[2].position = { extents_.max.x, extents_.min.y, extents_.min.z, 1.F };
		vertices_[3].position = { extents_.max.x, extents_.max.y, extents_.min.z, 1.F };
		vertices_[4].position = { extents_.max.x, extents_.max.y, extents_.min.z, 1.F };
		vertices_[5].position = { extents_.min.x, extents_.max.y, extents_.min.z, 1.F };
		vertices_[6].position = { extents_.min.x, extents_.min.y, extents_.min.z, 1.F };
		vertices_[7].position = { extents_.min.x, extents_.max.y, extents_.min.z, 1.F };
		vertices_[8].position = { extents_.min.x, extents_.min.y, extents_.max.z, 1.F };
		vertices_[9].position = { extents_.max.x, extents_.min.y, extents_.max.z, 1.F };
		vertices_[10].position = { extents_.max.x, extents_.min.y, extents_.max.z, 1.F };
		vertices_[11].position = { extents_.max.x, extents_.max.y, extents_.max.z, 1.F };
		vertices_[12].position = { extents_.max.x, extents_.max.y, extents_.max.z, 1.F };
		vertices_[13].position = { extents_.min.x, extents_.max.y, extents_.max.z, 1.F };
		vertices_[14].position = { extents_.min.x, extents_.min.y, extents_.max.z, 1.F };
		vertices_[15].position = { extents_.min.x, extents_.max.y, extents_.max.z, 1.F };
		vertices_[16].position = { extents_.min.x, extents_.min.y, extents_.min.z, 1.F };
		vertices_[17].position = { extents_.min.x, extents_.min.y, extents_.max.z, 1.F };
		vertices_[18].position = { extents_.max.x, extents_.min.y, extents_.min.z, 1.F };
		vertices_[19].position = { extents_.max.x, extents_.min.y, extents_.max.z, 1.F };
		vertices_[20].position = { extents_.min.x, extents_.max.y, extents_.min.z, 1.F };
		vertices_[21].position = { extents_.min.x, extents_.max.y, extents_.max.z, 1.F };
		vertices_[22].position = { extents_.max.x, extents_.max.y, extents_.min.z, 1.F };
		vertices_[23].position = { extents_.max.x, extents_.max.y, extents_.max.z, 1.F };
	}
}
