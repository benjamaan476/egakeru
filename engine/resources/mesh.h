#pragma once
#include "pch.h"

#include "geometry.h"
#include "transform.h"
#include "debug/debug_box3d.h"

namespace egkr
{
	class mesh : public resource, public std::enable_shared_from_this<mesh>
	{
	public:

		struct configuration
		{
			std::string name{};
			egkr::vector<geometry::properties> geometry_configurations{};
		};

		using shared_ptr = std::shared_ptr<mesh>;
		using weak_ptr = std::weak_ptr<mesh>;
		static shared_ptr create(const configuration& configuration);
		//static shared_ptr create(const geometry::geometry::shared_ptr& geometry, const transform& model);

		explicit mesh(const configuration& configration);
		~mesh();

		void load();
		void unload();

		void add_geometry(const geometry::geometry::shared_ptr& geometry);
		[[nodiscard]] const auto& get_geometries() const { return geometries_; }

		void set_model(const transform& model);
		[[nodiscard]] const auto& get_model() const { return model_; }

		[[nodiscard]] auto& extents() {	return extents_; }
		auto& model() {	return model_; }

		[[nodiscard]] auto& get_debug_data() { return debug_data; }
		[[nodiscard]] auto& unique_id() { return unique_id_; }

	private:
		void load_from_resource(std::string_view name);
	private:
		configuration configuration_{};
		egkr::vector<geometry::geometry::shared_ptr> geometries_{};
		transform model_;
		extent3d extents_{};
		//TODO how to do this properly?
		debug::debug_box3d::shared_ptr debug_data{};
		uint32_t unique_id_{};
	};
}
