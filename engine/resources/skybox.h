#pragma once
#include <pch.h>
#include <resources/resource.h>

#include <resources/texture.h>
#include <resources/geometry.h>

namespace egkr
{
	class skybox : public resource
	{
	public:
		struct configuration
		{
			std::string name{};
			texture_map::properties texture_map_properties{};
			geometry::properties geometry_properties{};
		};
		using shared_ptr = std::shared_ptr<skybox>;
		static shared_ptr create(const configuration& configuration);

		explicit skybox(const configuration& configuration);

		bool load();
		bool unload();
		~skybox() = default;

		void destroy();

		[[nodiscard]] auto get_geometry() const { return geometry_;}
		[[nodiscard]] auto& get_texture_map() const { return cubemap_;}
		[[nodiscard]] auto get_frame_number() const { return render_frame_number_;}
		[[nodiscard]] auto get_instance_id() const { return instance_id_;}

		void set_frame_number(uint64_t frame_number);

	private:
		configuration configuration_{};
		uint32_t instance_id_{0};
		texture_map::shared_ptr cubemap_{};
		geometry::shared_ptr geometry_{};
		uint64_t render_frame_number_{invalid_64_id};
	};

}
