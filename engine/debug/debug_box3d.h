#pragma once
#include <pch.h>
#include <resources/transform.h>
#include <resources/geometry.h>
#include <transformable.h>

namespace egkr::debug
{
	class debug_box3d : public transformable
	{
	public:
		using shared_ptr = std::shared_ptr<debug_box3d>;
		static shared_ptr create(const egkr::float3& size, egkr::transformable* parent);

		debug_box3d(const egkr::float3& size, egkr::transformable* parent);
		~debug_box3d();

		bool init();
		bool load();
		bool unload();
		bool update();

		void destroy();
		void set_colour(const egkr::float4 colour);
		void set_extents(const egkr::extent3d& extent);

		[[nodiscard]] const auto& get_geometry() const { return geometry_; }
		[[nodiscard]] const auto& get_id() const { return unique_id_; }
	private:
		void recalculate_colour();
		void recalculate_extents();
	private:
		uint32_t unique_id_{invalid_32_id};
		std::string name_{};
		egkr::float3 size_{};
		egkr::extent3d extents_{};
		egkr::float4 colour_{};
		egkr::vector<colour_vertex_3d> vertices_{};
		egkr::geometry::geometry::shared_ptr geometry_{};
	};
}
