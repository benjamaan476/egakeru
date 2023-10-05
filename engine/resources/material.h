#pragma once
#include "pch.h"

#include "resource.h"
#include "texture.h"

namespace egkr
{
	constexpr static std::string_view default_material_name_{ "default" };
	class material : public resource<material>
	{
	public:
		using shared_ptr = std::shared_ptr<material>;
		static shared_ptr create(const void* renderer_context);

		material();
		~material() = default;

		void set_name(const std::string& name) { name_ = name; }

		void set_diffuse_colour(const float4 diffuse);
		[[nodiscard]] const auto& get_diffuse_colour() const { return diffuse_colour_; }

		void set_internal_id(uint32_t id) { internal_id_ = id; }
		[[nodiscard]] const auto& get_internal_id() const { return internal_id_; }

		void set_diffuse_map(const texture_map& map) { diffuse_map_ = map; }
		[[nodiscard]] const auto& get_diffuse_map() const { return diffuse_map_; }
	private:
		uint32_t internal_id_{invalid_id};
		std::string name_{default_material_name_};

		float4 diffuse_colour_{1.F};
		texture_map diffuse_map_{};
	};
}
