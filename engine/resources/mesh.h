#pragma once
#include "pch.h"

#include "geometry.h"
#include "transform.h"

namespace egkr
{
	class mesh : public resource
	{
	public:
		using shared_ptr = std::shared_ptr<mesh>;
		static shared_ptr create();
		static shared_ptr create(const geometry::shared_ptr& geometry, const transform& model);

		mesh();

		void add_geometry(const geometry::shared_ptr& geometry);
		[[nodiscard]] const auto& get_geometries() const { return geometries_; }

		void set_model(const transform& model);
		[[nodiscard]] const auto& get_model() const { return model_; }
		auto& model() { return model_; }
	private:
		egkr::vector<geometry::shared_ptr> geometries_{};
		transform model_;
	};
}