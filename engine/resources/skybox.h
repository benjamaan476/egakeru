#pragma once
#include <pch.h>
#include <resources/resource.h>

#include <resources/texture.h>
#include <resources/geometry.h>

namespace egkr::skybox
{
	class skybox : public resource
	{
	public:
		using shared_ptr = std::shared_ptr<skybox>;
		shared_ptr create();

		skybox();
		~skybox();

		auto get_geometry() const { return geometry_;}
		auto get_texture_map() const { return cubemap_;}
		auto get_frame_number() const { return render_frame_number_;}

		void set_frame_number(uint64_t frame_number);

	private:
		uint32_t instance_id_{invalid_32_id};
		egkr::texture_map::texture_map::shared_ptr cubemap_{};
		egkr::geometry::geometry::shared_ptr geometry_{};
		uint64_t render_frame_number_{};

	};

}
