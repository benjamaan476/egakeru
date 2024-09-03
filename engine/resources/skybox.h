#pragma once
#include <pch.h>
#include <resources/resource.h>

#include <resources/texture.h>
#include <interfaces/renderable.h>

namespace egkr
{
	class skybox : public resource, public renderable
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

		[[nodiscard]] auto& get_texture_map() const { return cubemap_;}
		[[nodiscard]] auto get_frame_number() const { return render_frame_number_;}
		[[nodiscard]] auto get_draw_index() const { return draw_index_;}
		[[nodiscard]] auto get_instance_id() const { return instance_id_;}

		void set_frame_number(uint64_t frame_number);
		void set_draw_index(uint64_t draw_index);

	private:
		configuration configuration_{};
		uint32_t instance_id_{0};
		texture_map::shared_ptr cubemap_{};
		uint64_t render_frame_number_{invalid_64u_id};
		uint64_t draw_index_{invalid_64u_id};
	};

}
