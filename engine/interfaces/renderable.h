#pragma once
#include "resources/geometry.h"

namespace egkr
{
	struct renderable
	{
		renderable() = default;
		renderable(const geometry::shared_ptr& geometry) : geometry_{geometry} {}

		[[nodiscard]] const auto& get_geometry() const { return geometry_; }

		[[nodiscard]] const auto& get_material() const { return geometry_->get_material();}
		void set_material(const material::shared_ptr& material) { geometry_->set_material(material); }

		[[nodiscard]] const auto& get_properties() const { return geometry_->get_properties(); }

		void draw() const { geometry_->draw(); }
	protected:
		geometry::shared_ptr geometry_{};
    };
}
