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
		static shared_ptr create();

		explicit skybox();
		~skybox() = default;

		void destroy();

		auto get_geometry() const { return geometry_;}
		auto& get_texture_map() const { return cubemap_;}
		auto get_frame_number() const { return render_frame_number_;}
		auto get_instance_id() const { return instance_id_;}

		void set_frame_number(uint64_t frame_number);
		void set_geometry(geometry::geometry::shared_ptr geo);

	private:
		uint32_t instance_id_{0};
		texture_map::texture_map::shared_ptr cubemap_{};
		geometry::geometry::shared_ptr geometry_{};
		uint64_t render_frame_number_{invalid_64_id};

	};

}
