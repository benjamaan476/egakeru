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
		std::string name{};
		std::string diffuse_map_name{};
		material_type type{};
		float4 diffuse_colour{};
	};

	class renderer_frontend;
	class material : public resource
	{
	public:
		using shared_ptr = std::shared_ptr<material>;
		static shared_ptr create(const renderer_frontend* renderer, const material_properties& properties);

		explicit material(const material_properties& properties);
		~material() = default;

		void set_diffuse_colour(const float4 diffuse);
		[[nodiscard]] const auto& get_diffuse_colour() const { return diffuse_colour_; }

		void set_diffuse_map(const texture_map& map) { diffuse_map_ = map; }
		[[nodiscard]] const auto& get_diffuse_map() const { return diffuse_map_; }

		const auto& get_material_type() const { return type_; }

	private:
		float4 diffuse_colour_{1.F};
		texture_map diffuse_map_{};

		material_type type_{};

	};
}
