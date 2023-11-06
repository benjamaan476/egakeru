#pragma once
#include "pch.h"


#include "resource.h"
#include "texture.h"

namespace egkr
{
	constexpr static std::string_view default_material_name_{ "default" };

	enum class material_type
	{
		world,
		ui
	};

	struct material_properties
	{
		std::string name{"default"};
		std::string diffuse_map_name{"default_diffuse_texture"};
		std::string specular_map_name{"default_specular_texture"};
		std::string normal_map_name{"default_normal_texture"};
		std::string shader_name{"Shader.Builtin.Material"};
		float shininess{};
		float4 diffuse_colour{};
	};

	class renderer_frontend;
	class material : public resource
	{
	public:
		using shared_ptr = std::shared_ptr<material>;
		static shared_ptr create(const renderer_frontend* renderer, const material_properties& properties);

		explicit material(const renderer_frontend* renderer, const material_properties& properties);
		~material();

		void set_diffuse_colour(const float4 diffuse);
		[[nodiscard]] const auto& get_diffuse_colour() const { return diffuse_colour_; }

		void set_diffuse_map(const texture::texture_map& map) { diffuse_map_ = map; }
		[[nodiscard]] const auto& get_diffuse_map() const { return diffuse_map_; }
		[[nodiscard]] auto& get_diffuse_map() { return diffuse_map_; }

		void set_specular_map(const texture::texture_map& map) { specular_map_ = map; }
		[[nodiscard]] const auto& get_specular_map() const { return specular_map_; }
		[[nodiscard]] auto& get_specular_map() { return specular_map_; }

		void set_normal_map(const texture::texture_map& map) { normal_map_ = map; }
		[[nodiscard]] const auto& get_normal_map() const { return normal_map_; }
		[[nodiscard]] auto& get_normal_map() { return normal_map_; }

		void set_shininess(float shininess) { shininess_ = shininess; }
		const auto& get_shininess() const { return shininess_; }

		const auto& get_shader_id() const { return shader_id_; }
		const auto& get_shader_name() const { return shader_name_; }

		const auto& get_internal_id() const {return internal_id_; }
		void set_internal_id(uint32_t id) { internal_id_ = id; }

		[[nodiscard]] auto get_render_frame() const { return render_frame_number_; }
		void set_render_frame(uint32_t frame) { render_frame_number_ = frame; }
	private:
		const renderer_frontend* renderer_{};
		float shininess_{32.F};
		float4 diffuse_colour_{1.F};
		texture::texture_map diffuse_map_{};
		texture::texture_map specular_map_{};
		texture::texture_map normal_map_{};

		std::string shader_name_{};
		uint32_t shader_id_{invalid_32_id};
		uint32_t internal_id_{ invalid_32_id };
		uint32_t render_frame_number_{ invalid_32_id };
	};
}
